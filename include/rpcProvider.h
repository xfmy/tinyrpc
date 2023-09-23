#pragma once
#include "mprpcHeader.pb.h"
#include <google/protobuf/service.h>
#include <unordered_map>
#include <muduo/net/TcpConnection.h>

namespace PROTO = google::protobuf;

// 框架提供的专门发布rpc服务的网络对象类
class RpcProvider
{
public:
    RpcProvider();
    // 这是框架提供个给外部使用的,可以发布rpc方法的函数接口
    void NotifyService(google::protobuf::Service *service);

    // 启动rpc服务
    void run();
private:
    class ServiceInfo
    {
    public:
        const PROTO::Service *GetService() const;
        const PROTO::MethodDescriptor *GetMethodDescriptor(const std::string) const;
        void SetService(PROTO::Service *);
        void AddMethodDescripto(const std::string, const PROTO::MethodDescriptor *);
    private:
        PROTO::Service *m_service;
        std::unordered_map<std::string, const PROTO::MethodDescriptor *> m_methodMap;
    };
    //网络层数据发送完毕回调
    void writeComplete(const muduo::net::TcpConnectionPtr &);
    
    //rpc解析分发回调方法
    void serviceDistribute(const TcpConnectionPtr &, const std::string &, Timestamp);
    
    //解析rpcHeadler信息
    mprpcHeader::rpcHeader parseHeadler(const std::string &data);
    
    //客户端响应回调函数
    void responseToClient(const TcpConnectionPtr& , PROTO::Message* response);

    // 解析request请求信息,并将protobuf解析数据写入PROTO::Message * req
    void parseRequast(PROTO::Message* req, const std::string &data, uint32_t argsSize);

    //获取service信息
    const ServiceInfo &GetServiceInfo(std::string serviceName) const;
private:
    //服务名与服务信息映射表
    std::unordered_map<std::string, ServiceInfo> m_serviceMap;
};