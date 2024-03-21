/**
 * @file rpcProvider.h
 * @brief 框架提供的专门发布rpc服务的网络对象模块
 * @author xf
 * @version 1.0
 * @copyright BSD
 * @date 2023-11-27
 */

#pragma once
#include <unordered_map>
#include <muduo/net/TcpConnection.h>
#include <muduo/base/Timestamp.h>
#include <google/protobuf/service.h>
//#include "consul.h"

namespace PROTO = google::protobuf;
class google::protobuf::Message;
using servicePtr = std::shared_ptr<google::protobuf::Service>;
using messagePtr = std::shared_ptr<google::protobuf::Message>;

namespace mprpc {
class TinyPBProtocol;
class ConsulClient;
    /// @brief 框架提供的专门发布rpc服务的网络对象类
    class RpcDispatcher
{
public:
    RpcDispatcher();

    /// @brief 这是框架提供个给外部使用的,可以发布rpc方法的函数接口
    /// @param service 需要发布的服务类
    void registerService(servicePtr service);

    /// @brief 启动rpc服务
    void run();

private:
    // class ServiceInfo
    // {
    // public:
    //     const PROTO::Service *GetService() const;
    //     const PROTO::MethodDescriptor *GetMethodDescriptor(
    //         const std::string) const;
    //     void SetService(PROTO::Service *);
    //     void AddMethodDescripto(const std::string,
    //                             const PROTO::MethodDescriptor *);

    // private:
    //     PROTO::Service *m_service;
    //     std::unordered_map<std::string, const PROTO::MethodDescriptor *>
    //         m_methodMap;
    // };
    //网络层数据发送完毕回调
    void writeComplete(const muduo::net::TcpConnectionPtr &);

    // rpc解析分发回调方法
    // void serviceDistribute(const muduo::net::TcpConnectionPtr &,
    //                       const std::string &, muduo::Timestamp);
    void dispatch(std::shared_ptr<TinyPBProtocol> reqest,
                  std::shared_ptr<TinyPBProtocol> response,
                  const muduo::net::TcpConnectionPtr &ptr);

    //解析rpcHeadler信息
    // mprpcHeader::rpcHeader parseHeadler(const std::string &data);

    //客户端响应回调函数
    void responseToClient(const muduo::net::TcpConnectionPtr &,
                          std::shared_ptr<TinyPBProtocol> response);

    // 解析request请求信息,并将protobuf解析数据写入PROTO::Message * req
    // void parseRequast(PROTO::Message *req, const std::string &data,
    //                   uint32_t argsSize);

    //获取service信息
    //const ServiceInfo &GetServiceInfo(std::string serviceName) const;

    //解析服务名与方法名
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
    //服务名与服务信息映射表
    // std::unordered_map<std::string, ServiceInfo> m_serviceMap;
    std::unordered_map<std::string, servicePtr> serviceMap_;
    std::shared_ptr<ConsulClient> consulPtr_;
    std::vector<std::string> services_;
};
} // namespace mprpc