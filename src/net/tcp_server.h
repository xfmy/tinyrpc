/**
 * @file tinyrpcNetwork.h
 * @brief rpc server网络模块,封装muduo网络库TcpServer
 */

#pragma once
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <memory>
#include <string_view>
#include "thread_pool.h"
using namespace muduo;
using namespace muduo::net;

typedef std::function<void(EventLoop *)> ThreadInitCallback;

namespace tinyrpc {

class TinyPBProtocol;

using DispatchCallback =
    std::function<void(std::shared_ptr<TinyPBProtocol> reqest,
                       std::shared_ptr<TinyPBProtocol> response,
                       const muduo::net::TcpConnectionPtr &ptr)>;

/// @brief tinyrpc的网络模块
class TcpServer : public muduo::noncopyable
{
public:
    TcpServer(uint16_t port);
    ~TcpServer();

    /// @brief 启动网络模块
    void start();

    /// @brief 设置数据包包发送完毕的回调函数
    void setWriteCompleteCallback(const WriteCompleteCallback cb)
    {
        server_.setWriteCompleteCallback(cb);
    }

    /// @brief 设置客户端连接回调
    void setConnectCallback(const ConnectionCallback cb)
    {
        server_.setConnectionCallback(cb);
    }

    /// @brief 设置接收到一个完整的包之后调用的业务处理函数
    void setDispatchCallback(const DispatchCallback &cb)
    {
        dispatchCallback_ = cb;
    }

    /// @brief 添加定时器
    /// @param delay 时间
    /// @param cb 事件回调
    void AddTimerEvent(double delay, TimerCallback cb);

private:
    /// @brief 连接回调函数
    void onConnectCallback(const TcpConnectionPtr &ptr);

    /// @brief 设置workLoop数量
    void setThreadNum(int count) { server_.setThreadNum(count); }

    /// @brief 设置线程初始化完毕的回调函数
    void setThreadInitCallback(const ThreadInitCallback cb)
    {
        server_.setThreadInitCallback(cb);
    }

    /// @brief 消息回调
    void onMessageCallback(const TcpConnectionPtr &ptr, Buffer *buf, Timestamp);

    /// @brief workLoop初始化回调函数
    void onThreadInitCallback(EventLoop *);

    /// @brief 设置业务层的消息处理回调函数
    DispatchCallback dispatchCallback_;

    const std::string serverName_ = "networkServer";
    InetAddress addr_;
    EventLoop loop_;
    muduo::net::TcpServer server_;

    /// @brief 业务线程池
    std::unique_ptr<CThreadPool> pool_;
};
} // namespace tinyrpc