/**
 * @file rpc_channel.h
 * @author xf
 * @brief channel类继承Google::protobuf::RpcChannel.
 *          通过重写基类的CallMethod方法实现客户端的序列化以及网络发送
 */

#pragma once
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>
#include <muduo/net/InetAddress.h>

namespace PROTO = google::protobuf;
using namespace muduo;
using namespace muduo::net;

namespace tinyrpc {
class TcpClient;
class TinyPBProtocol;
/// @brief 由rpc调用端使用的通道类
class RpcChannel : public google::protobuf::RpcChannel,
                   public std::enable_shared_from_this<RpcChannel>
{
public:
    using RpcChannelPtr = std::shared_ptr<RpcChannel>;
    using ControllerPtr = std::shared_ptr<google::protobuf::RpcController>;
    using MessagePtr = std::shared_ptr<google::protobuf::Message>;
    using ClosurePtr = std::shared_ptr<google::protobuf::Closure>;
    using TcpClientPtr = std::shared_ptr<tinyrpc::TcpClient>;

public:
    RpcChannel();

    ~RpcChannel();

    /// @brief 虚函数,编写rpc远程调用时具体逻辑处理
    /// @param method rpc请求的方法描述
    /// @param controller 控制类
    /// @param request 请求消息类
    /// @param response 响应消息类
    /// @param done 具体执行调用方法
    void CallMethod(const google::protobuf::MethodDescriptor* method,
                    google::protobuf::RpcController* controller,
                    const google::protobuf::Message* request,
                    google::protobuf::Message* response,
                    google::protobuf::Closure* done);

private:
    /// @brief tcp client网络模块
    TcpClientPtr client_{nullptr};
    /// @brief 初始化标记
    bool init_ = false;
};

} // namespace tinyrpc
