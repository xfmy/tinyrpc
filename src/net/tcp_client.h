/**
 * @file tcp_client.h
 * @author xf
 * @brief 封装muduo tcp client通信
 */
#pragma once
#include <memory>
#include <map>
#include <mutex>
#include <condition_variable>
#include "muduo/net/TcpClient.h"
#include "muduo/net/InetAddress.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/EventLoopThread.h"

// namespace PROTO = google::protobuf;
using namespace muduo;
using namespace muduo::net;

// using TcpClientPtr = std::shared_ptr<muduo::net::TcpClient>;
// using InetAddrPtr = std::shared_ptr<muduo::net::InetAddress>;

namespace tinyrpc {
class TinyPBProtocol;
class RpcController;
class TcpClient
{
public:
    TcpClient(muduo::net::InetAddress peer_addr);
    ~TcpClient();

    // 将TinyPB数据进行序列化后向服务端发送请求
    void request(std::shared_ptr<tinyrpc::TinyPBProtocol> req_protocol);

    // 添加定时器
    void AddTimerEvent(double delay, TimerCallback cb);

    // 连接是否成功
    bool connected();

    // 获取服务端地址的字符串
    std::string GetPeerAddrString();

    // 获取TingPB数据
    bool GetTinyPBProtocol(RpcController*,
                           std::shared_ptr<tinyrpc::TinyPBProtocol>&);

private:
    // 接收消息回调
    void MessageCallback(const muduo::net::TcpConnectionPtr&, Buffer*,
                         Timestamp);
    // 链接回调
    void connectCallbcak(const TcpConnectionPtr&);
    // void SetTinyPBProtocol(std::shared_ptr<tinyrpc::TinyPBProtocol>);

private:
    /// @brief 本机地址
    muduo::net::InetAddress LocalAddr_;
    /// @brief 对端地址 
    muduo::net::InetAddress peerAddr_;
    /// @brief 事件循环线程类
    muduo::net::EventLoopThread loopThread_;
    /// @brief 事件循环
    muduo::net::EventLoop* loop_;
    /// @brief tcp客户端类
    muduo::net::TcpClient tcpClient_;
    /// @brief 连接管理类
    TcpConnectionPtr connectionPtr;

    //管理服务端响应的数据包集合
    std::map<std::string, std::shared_ptr<tinyrpc::TinyPBProtocol>> tingPBMap_;
    std::mutex mtx_;
    std::condition_variable cv_;
};
} // namespace tinyrpc