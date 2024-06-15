/**
 * @file rpcProvider.h
 * @brief 框架提供的专门发布rpc服务与服务分发的网络对象模块
 *
 */

#pragma once
#include <unordered_map>
#include <muduo/net/TcpConnection.h>
#include <muduo/base/Timestamp.h>
#include <google/protobuf/service.h>

namespace PROTO = google::protobuf;
//class google::protobuf::Message;
using servicePtr = std::shared_ptr<google::protobuf::Service>;
using messagePtr = std::shared_ptr<google::protobuf::Message>;

namespace tinyrpc {
class TinyPBProtocol;
class ConsulClient;
/// @brief 框架提供的专门发布rpc服务与服务分发的网络对象模块
class RpcDispatcher
{
public:
    RpcDispatcher();
    ~RpcDispatcher();

    /// @brief 这是框架提供个给外部使用的,可以发布rpc方法的函数接口
    /// @param service 需要发布的服务类
    void registerService(servicePtr service);

    /// @brief 启动rpc服务
    void run();

private:
    /// @brief 网络层数据发送完毕回调
    void writeComplete(const muduo::net::TcpConnectionPtr &);

    /// @brief 客户端响应回调函数
    /// @param response TinyPBProtocol响应信息
    void responseToClient(const muduo::net::TcpConnectionPtr &,
                          std::shared_ptr<TinyPBProtocol> response);

    /// @brief 分发rpc消息,调用具体的rpc服务
    /// @param reqest 客户端请求的tinypb信息
    /// @param response 服务端响应的tinypb信息
    /// @param ptr muduo tcp网络链接类
    void dispatch(std::shared_ptr<TinyPBProtocol> reqest,
                  std::shared_ptr<TinyPBProtocol> response,
                  const muduo::net::TcpConnectionPtr &ptr);

    /// @brief 解析服务名与方法名
    /// @param full_name rpc服务完整名称
    /// @param service_name 出参,解析出的服务名
    /// @param method_name 出参,解析出的方法名
    /// @return 解析是否成功
    bool parseServiceFullName(const std::string &full_name,
                              std::string &service_name,
                              std::string &method_name);
    /// @brief 设置tinyPB协议错误
    /// @param msg response tinyPB
    /// @param err_code 错误码
    /// @param err_info 错误信息
    void setTinyPBError(std::shared_ptr<TinyPBProtocol> msg, int32_t err_code,
                        const std::string err_info);

private:
    /// @brief 服务名与服务信息映射表
    std::unordered_map<std::string, servicePtr> serviceMap_;

    /// @brief consul操作类指针
    std::shared_ptr<ConsulClient> consulPtr_;

    /// @brief consul注册名列表
    std::vector<std::string> services_;
};
} // namespace tinyrpc