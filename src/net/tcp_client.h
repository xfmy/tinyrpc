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

namespace mprpc {
class TinyPBProtocol;
class RpcController;
class TcpClient
{
public:
    TcpClient(muduo::net::InetAddress peer_addr);
    ~TcpClient();

    void request(std::shared_ptr<mprpc::TinyPBProtocol> req_protocol);
    void AddTimerEvent(double delay, TimerCallback cb);
    bool connected();
    std::string GetPeerAddrString();

    bool GetTinyPBProtocol(RpcController*,
                           std::shared_ptr<mprpc::TinyPBProtocol>&);

private:
    void MessageCallback(const muduo::net::TcpConnectionPtr&, Buffer*,
                         Timestamp);
    void connectCallbcak(const TcpConnectionPtr&);
    // void SetTinyPBProtocol(std::shared_ptr<mprpc::TinyPBProtocol>);

private:
    muduo::net::InetAddress LocalAddr_;
    muduo::net::InetAddress peerAddr_;
    // muduo::net::EventLoop loop_;
    muduo::net::EventLoopThread loop_;
    muduo::net::TcpClient tcpClient_;
    TcpConnectionPtr connectionPtr;
    std::map<std::string, std::shared_ptr<mprpc::TinyPBProtocol>> tingPBMap_;

    std::mutex mtx_;
    std::condition_variable cv_;
};
} // namespace mprpc