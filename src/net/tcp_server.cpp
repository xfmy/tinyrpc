#include <string_view>
#include <string>
#include <optional>
#include <thread>
#include <functional>
#include <nlohmann/json.hpp>
#include <muduo/base/TimeZone.h>
#include <muduo/base/Logging.h>

#include "tcp_server.h"
#include "tinyPB_coder.h"
#include "tinyPB_protocol.h"

namespace tinyrpc {
TcpServer::TcpServer(uint16_t port)
    : addr_(port),
      loop_(),
      server_(&loop_, addr_, serverName_)
{
    // 设置东八区
    TimeZone time(8 * 1000 * 60 * 60, "loaclTime");
    Logger::setTimeZone(time);

    // 设置tcpserver的网络层相关回调
    server_.setMessageCallback(
        std::bind(&TcpServer::onMessageCallback, this, _1, _2, _3));
    server_.setThreadInitCallback(
        std::bind(&TcpServer::onThreadInitCallback, this, _1));
    server_.setConnectionCallback(

        std::bind(&TcpServer::onConnectCallback, this, _1));
    setThreadNum(std::thread::hardware_concurrency()*2);

    //创建并初始化线程池
    pool_ = std::make_unique<CThreadPool>();
    pool_->SetMode(CPoolMode::MODE_FIXED);
    pool_->Start();
}


TcpServer::~TcpServer() {}

void TcpServer::start()
{
    server_.start();
    loop_.loop();
}

void TcpServer::onMessageCallback(const TcpConnectionPtr &ptr, Buffer *buf,
                                      Timestamp time)
{
    // 首先解析一个完整的包,然后调用onMessageCompleteCallback处理
    std::string_view view(buf->peek(), buf->readableBytes());
    LOG_INFO << "接收到一个的包 data size:" << view.size();
    std::shared_ptr<tinyrpc::TinyPBProtocol> request =
        std::make_shared<tinyrpc::TinyPBProtocol>();
    std::shared_ptr<tinyrpc::TinyPBProtocol> response =
        std::make_shared<tinyrpc::TinyPBProtocol>();
    // 解析包数据
    int index = tinyrpc::TinyPBCoder::decode(request,view);
    if (index == -1)
        return;
    else
    {
        // 将已经解析使用的数据接收缓冲区释放
        buf->retrieve(index);
        // 将业务处理添加至线程池执行
        pool_->AddTask(dispatchCallback_, request, response, ptr);
    }
}

void TcpServer::AddTimerEvent(double delay, TimerCallback cb) 
{
    loop_.runEvery(delay,cb);
}

void TcpServer::onConnectCallback(const TcpConnectionPtr &ptr)
{
    if (ptr->connected())
    {
        LOG_INFO << ptr->peerAddress().toIpPort() + "客户发起来了连接";
    }
    else if (ptr->disconnected())
    {
        LOG_INFO << ptr->peerAddress().toIpPort() + "客户断开了连接";
        ptr->shutdown();
    }
}

void TcpServer::onThreadInitCallback(EventLoop *)
{
    LOG_INFO << "work loop thread start";
}
}