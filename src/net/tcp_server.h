/**
 * @mainpage 网络层
 * @file mprpcNetwork.h
 * @brief 整个项目的网络模块,封装muduo网络库TcpServer
 * @author xf
 * @version 1.0
 * @copyright BSD
 * @date 2023-11-27
 */

#pragma once
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <memory>
#include <string_view>
using namespace muduo;
using namespace muduo::net;

typedef std::function<void(EventLoop *)> ThreadInitCallback;

// using businessCallback = std::function<void(const TcpConnectionPtr &,
//                                             const std::string &,
//                                             Timestamp)>;
// typedef std::function<void(const TcpConnectionPtr &, const std::string &,
// Timestamp)> businessCallback;
// using packageFullCallback =
//    std::function<int(std::string_view view, std::string &target)>;

namespace mprpc {

class TinyPBProtocol;

using DispatchCallback =
    std::function<void(std::shared_ptr<TinyPBProtocol> reqest,
                       std::shared_ptr<TinyPBProtocol> response,
                       const muduo::net::TcpConnectionPtr &ptr)>;

/// @brief mprpc的网络模块
class TcpServer : public muduo::noncopyable
{
public:
    TcpServer(uint16_t port);
    ~TcpServer();

    /// @brief 启动
    void start();

    /// @brief 设置包发送完毕的回调函数
    void setWriteCompleteCallback(const WriteCompleteCallback cb)
    {
        server_.setWriteCompleteCallback(cb);
    }

    /// @brief 设置客户端连接完毕后的回调函数
    void setConnectCallback(const ConnectionCallback cb)
    {
        server_.setConnectionCallback(cb);
    }

    /// @brief 设置接收到一个完整的包之后调用的业务处理函数
    // void setBusinessMessageCallback(const businessCallback cb)
    // {
    //     this->businessMsgCallback = cb;
    // }
    void setDispatchCallback(const DispatchCallback& cb) { dispatchCallback_ = cb;}
    void AddTimerEvent(double delay, TimerCallback cb);

private:
    // 设置验证包完整性的回调函数
    // void setPackageFullCallback(const packageFullCallback cb) { packageFull =
    // cb; };

    //设置连接回调函数
    void onConnectCallback(const TcpConnectionPtr &ptr);

    // 设置workLoop数量
    void setThreadNum(int count) { server_.setThreadNum(count); }

    //设置线程初始化完毕的回调函数
    void setThreadInitCallback(const ThreadInitCallback cb)
    {
        server_.setThreadInitCallback(cb);
    }

    // muduo消息回调
    void onMessageCallback(const TcpConnectionPtr &ptr, Buffer *buf, Timestamp);

    // workLoop初始化回调函数
    void onThreadInitCallback(EventLoop *);

    // 设置业务层的消息处理回调函数
    // businessCallback businessMsgCallback;
    DispatchCallback dispatchCallback_;

    //设置包的完整性验证回调
    // packageFullCallback packageFull;

    const std::string serverName_ = "networkServer";
    InetAddress addr_;
    EventLoop loop_;
    muduo::net::TcpServer server_;
};
} // namespace mprpc