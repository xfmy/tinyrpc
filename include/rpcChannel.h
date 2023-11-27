#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>

namespace PROTO = google::protobuf;

/// @brief 由rpc调用端使用的通道类
class RpcChannel : public PROTO::RpcChannel
{
public:
    /// @brief 虚函数,编写rpc远程调用时具体逻辑处理
    /// @param method rpc请求的方法描述
    /// @param controller 控制类
    /// @param request 请求消息类
    /// @param response 响应消息类
    /// @param done 具体执行调用方法
    virtual void CallMethod(const PROTO::MethodDescriptor *method,
                            PROTO::RpcController *controller,
                            const PROTO::Message *request,
                            PROTO::Message *response, 
                            PROTO::Closure *done) override;
};
