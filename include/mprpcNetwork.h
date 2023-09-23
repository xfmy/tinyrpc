// 网络层
#pragma once
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <memory>
#include <string_view>
using namespace muduo;
using namespace muduo::net;

typedef std::function<void(EventLoop *)> ThreadInitCallback;
using businessCallback = std::function<void(const TcpConnectionPtr &, const std::string &, Timestamp)>;
//typedef std::function<void(const TcpConnectionPtr &, const std::string &, Timestamp)> businessCallback;
using packageFullCallback = std::function<int(std::string_view view, std::string &target)>;

class rpcNetwork : public muduo::noncopyable
{
public:
    rpcNetwork(uint16_t port);
    ~rpcNetwork();
    
    void start();
    //void init();

    //设置包发送完毕的回调函数
    void setWriteCompleteCallback(const WriteCompleteCallback cb) { server.setWriteCompleteCallback(cb); }

    //设置客户端连接完毕后的回调函数
    void setConnectCallback(const ConnectionCallback cb) { server.setConnectionCallback(cb); }

    //设置接收到一个完整的包之后调用的业务处理函数
    void setBusinessMessageCallback(const businessCallback cb) { this->businessMsgCallback = cb; }

    //设置验证包完整性的回调函数
    void setPackageFullCallback(const packageFullCallback cb) { packageFull = cb; };

private:
    // 设置workLoop数量
    void setThreadNum(int count) { server.setThreadNum(count); }
    
    //设置线程初始化完毕的回调函数
    void setThreadInitCallback(const ThreadInitCallback cb) { server.setThreadInitCallback(cb); }

    //muduo消息回调
    void onMessageCallback(const TcpConnectionPtr &ptr, Buffer *buf, Timestamp);

    //workLoop初始化回调函数
    void onThreadInitCallback(EventLoop *);

    // 设置业务层的消息处理回调函数
    businessCallback businessMsgCallback;

    //设置包的完整性验证回调
    packageFullCallback packageFull;

    const std::string serverName = "networkServer";
    InetAddress addr;
    EventLoop loop;
    TcpServer server;
};