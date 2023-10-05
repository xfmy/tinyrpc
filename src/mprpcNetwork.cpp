//#include "package.h"
#include "mprpcNetwork.h"
#include "package.h"
#include <nlohmann/json.hpp>
#include <muduo/base/TimeZone.h>
#include <muduo/base/Logging.h>
#include <string_view>
#include <string>
#include <optional>
#include <thread>
#include <functional>

// void rpcNetwork::init()
// {
//     // 设置tcpserver的网络层相关回调
//     server.setMessageCallback(std::bind(&rpcNetwork::onMessageCallback, this, _1, _2, _3));
//     server.setThreadInitCallback(std::bind(&rpcNetwork::onThreadInitCallback, this, _1));
// }

rpcNetwork::rpcNetwork(uint16_t port)
    : addr(port),
      loop(),
      server(&loop, addr, serverName)
{
    // 设置东八区
    TimeZone time(8 * 1000 * 60 * 60, "loaclTime");
    Logger::setTimeZone(time);

    // 设置tcpserver的网络层相关回调
    server.setMessageCallback(std::bind(&rpcNetwork::onMessageCallback, this, _1, _2, _3));
    server.setThreadInitCallback(std::bind(&rpcNetwork::onThreadInitCallback, this, _1));
    server.setConnectionCallback(std::bind(&rpcNetwork::onConnectCallback,this,_1));
    setThreadNum(std::thread::hardware_concurrency());
    setPackageFullCallback(&package::parse);
}

rpcNetwork::~rpcNetwork()
{
}

void rpcNetwork::start()
{
    server.start();
    loop.loop();
}

// void chatNetworkLayer::onWriteCompleteCallback(const TcpConnectionPtr &ptr)
// {
//     LOG_INFO <<ptr->peerAddress().toIpPort() + "消息发送完毕";
// }

void rpcNetwork::onMessageCallback(const TcpConnectionPtr &ptr, Buffer *buf, Timestamp time)
{
    // 首先解析一个完整的包,然后调用onMessageCompleteCallback处理
    std::string view(buf->peek(), buf->readableBytes());
    std::string userData;
    LOG_INFO << "接收到一个的包 data size:" << view.size();
    int index = packageFull(view, userData);
    if(index == -1)
        return;
    else{
        LOG_INFO <<"接收到一个完整的包";
        buf->retrieve(index);
        businessMsgCallback(ptr, userData, time);
    }
}

void rpcNetwork::onConnectCallback(const TcpConnectionPtr &ptr)
{
    if (ptr->connected()){
        LOG_INFO << ptr->peerAddress().toIpPort() + "客户发起来了连接";
    }
    else if(ptr->disconnected()){
        LOG_INFO << ptr->peerAddress().toIpPort() + "客户断开了连接";
        ptr->shutdown();
    }
}

void rpcNetwork::onThreadInitCallback(EventLoop *)
{
    // TODO:都需要处理
    LOG_INFO << "work loop thread start";
}
