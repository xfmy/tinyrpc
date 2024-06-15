#include "tcp_client.h"
#include "muduo/base/Logging.h"
#include "rpc_controller.h"
#include "tinyPB_coder.h"
#include "tinyPB_protocol.h"

namespace tinyrpc {
TcpClient::TcpClient(muduo::net::InetAddress peer_addr)
    : peerAddr_(peer_addr),
      loop_(loopThread_.startLoop()),
      tcpClient_(loop_, peerAddr_, "tcp_client")
{
    tcpClient_.setMessageCallback(
        std::bind(&TcpClient::MessageCallback, this, _1, _2, _3));
    tcpClient_.setConnectionCallback(
        std::bind(&tinyrpc::TcpClient::connectCallbcak, this, _1));
    tcpClient_.enableRetry();
    mtx_.lock();
    tcpClient_.connect();

    //保证网络已经连接完毕
    std::lock_guard<std::mutex> lock(mtx_);
}

TcpClient::~TcpClient() {}

void TcpClient::MessageCallback(const muduo::net::TcpConnectionPtr& ptr,
                                Buffer* buf, Timestamp)
{
    std::string_view view(buf->peek(), buf->readableBytes());
    LOG_INFO << "接收到一个的包 data size:" << view.size();

    std::shared_ptr<tinyrpc::TinyPBProtocol> response =
        std::make_shared<tinyrpc::TinyPBProtocol>();

    int index = tinyrpc::TinyPBCoder::decode(response, view);
    if (index == -1)
        return;
    else
    {
        // 释放已经解析的接收缓冲区数据
        buf->retrieve(index);

        // 将已经响应的rpc调用结果加入集合中,并唤醒消费者
        tingPBMap_.emplace(response->msgId_, response);
        cv_.notify_all();
    }
}
void TcpClient::connectCallbcak(const TcpConnectionPtr& ptr)
{
    connectionPtr = ptr;
    if (connectionPtr->connected()) peerAddr_ = connectionPtr->peerAddress();
    mtx_.unlock();
}
void TcpClient::request(std::shared_ptr<tinyrpc::TinyPBProtocol> req_protocol)
{
    std::string sendMsg;
    TinyPBCoder::encode(req_protocol, sendMsg);
    connectionPtr->send(sendMsg);
}
void TcpClient::AddTimerEvent(double delay, TimerCallback cb)
{
    loop_->runAfter(delay, cb);
}
bool TcpClient::connected() { return connectionPtr->connected(); }
std::string TcpClient::GetPeerAddrString() { return peerAddr_.toIpPort(); }
bool TcpClient::GetTinyPBProtocol(RpcController* controller,
                                  std::shared_ptr<tinyrpc::TinyPBProtocol>& ptr)
{
    // msgid+timeout
    std::string msgId = controller->GetMsgId();
    std::unique_lock<std::mutex> lock(mtx_);
    bool timeout =
        cv_.wait_for(lock, std::chrono::milliseconds(controller->GetTimeout()),
                     [this, msgId, &ptr] mutable -> bool {
                         // 通过msgId寻到数据包
                         auto it = tingPBMap_.find(msgId);
                         if (it != tingPBMap_.end())
                         {
                             ptr = tingPBMap_[msgId];
                             tingPBMap_.erase(it);
                             return true;
                         }
                         else
                         {
                             return false;
                         }
                     });
    return timeout;
}
} // namespace tinyrpc