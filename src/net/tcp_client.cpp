#include "tcp_client.h"
#include "muduo/base/Logging.h"
#include "rpc_controller.h"
#include "tinyPB_coder.h"
#include "tinyPB_protocol.h"

namespace mprpc {
TcpClient::TcpClient(muduo::net::InetAddress peer_addr)
    : peerAddr_(peer_addr),
      tcpClient_(loop_.startLoop(), peerAddr_, "tcp_client")
{
    tcpClient_.setMessageCallback(
        std::bind(&TcpClient::MessageCallback, this, _1, _2, _3));
    tcpClient_.setConnectionCallback(std::bind(&mprpc::TcpClient::connectCallbcak, this, _1));
    tcpClient_.enableRetry();
    mtx_.lock();
    tcpClient_.connect();
    std::lock_guard<std::mutex> lock(mtx_);

    //connectionPtr = tcpClient_.connection();

    // if (connectionPtr->connected()) LocalAddr_ =
    // connectionPtr->localAddress();
}

TcpClient::~TcpClient() {}

void TcpClient::MessageCallback(const muduo::net::TcpConnectionPtr& ptr,
                                Buffer* buf, Timestamp)
{
    std::string_view view(buf->peek(), buf->readableBytes());
    LOG_INFO << "接收到一个的包 data size:" << view.size();

    std::shared_ptr<mprpc::TinyPBProtocol> response =
        std::make_shared<mprpc::TinyPBProtocol>();

    int index = mprpc::TinyPBCoder::decode(response, view);
    if (index == -1)
        return;
    else
    {
        buf->retrieve(index);
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
void TcpClient::request(std::shared_ptr<mprpc::TinyPBProtocol> req_protocol)
{
    std::string sendMsg;
    TinyPBCoder::encode(req_protocol, sendMsg);
    connectionPtr->send(sendMsg);
}
void TcpClient::AddTimerEvent(double delay, TimerCallback cb)
{
    //loop_.runAfter(delay, cb);
}
bool TcpClient::connected() { return connectionPtr->connected(); }
std::string TcpClient::GetPeerAddrString() { return peerAddr_.toIpPort(); }
bool TcpClient::GetTinyPBProtocol(RpcController* controller,
                                  std::shared_ptr<mprpc::TinyPBProtocol>& ptr)
{
    // msgid+timeout
    std::string msgId = controller->GetMsgId();
    std::unique_lock<std::mutex> lock(mtx_);
    bool timeout =
        cv_.wait_for(lock, std::chrono::milliseconds(controller->GetTimeout()),
                    [this, msgId, &ptr] mutable -> bool {
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
} // namespace mprpc