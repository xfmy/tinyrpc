#include "rpcProvider.h"
#include "mprpcApplication.h"
#include "mprpcNetwork.h"
#include "protobuf.pb.h"
#include "except.hpp"
#include <google/protobuf/descriptor.h>
#include <muduo/base/Logging.h>

RpcProvider::RpcProvider()
{

}

void RpcProvider::NotifyService(PROTO::Service *service)
{
    ServiceInfo serviceINfo;

    //获取服务对象的描述信息
    const PROTO::ServiceDescriptor* pserviceDesc = service->GetDescriptor();
    //获取服务的名字
    std::string serviceName = pserviceDesc->name();
    //获取服务对象的service方法数量
    int methodCount = pserviceDesc->method_count();
    for (size_t i = 0; i < methodCount; i++)
    {
        //获取下标相应的方法描述
        const PROTO::MethodDescriptor* pmethodDesc = pserviceDesc->method(i);
        //h获取函数名
        std::string functionName = pmethodDesc->name();
        //将函数名与方法描述加入函数表中
        serviceINfo.AddMethodDescripto(functionName, pmethodDesc);
    }
    //添加服务表
    serviceINfo.SetService(service);
    m_serviceMap.emplace(serviceName,serviceINfo);
}

void RpcProvider::run()
{
    std::optional<std::string> servicePort = MprpcApplication::GetInstance().atConfigItem("serverPort");
    if(servicePort.has_value()){
        int port = std::atoi(servicePort.value().c_str());
        rpcNetwork networkLayer(port);
        //设置rpc分发回调函数
        networkLayer.setBusinessMessageCallback(std::bind(RpcProvider::serviceDistribute,this, _1, _2, _3));
        networkLayer.start();
    }
    else
    {
        LOG_FATAL << "serverPort not config";
        exit(-1);
    }
}

void RpcProvider::writeComplete(const muduo::net::TcpConnectionPtr& connectPtr)
{
    connectPtr->shutdown();
}

void RpcProvider::serviceDistribute(const TcpConnectionPtr& connectPtr, const std::string &data, Timestamp)
{
    try{
        //解析rpcHeadler头部并获取相应的字段
        mprpcHeader::rpcHeader headler = parseHeadler(data);
        std::string serviceName = headler.servicename();
        std::string methodName = headler.methodname();
        uint32_t argsSize = headler.argssize();

        //获取service信息
        const ServiceInfo& serviceINfo = GetServiceInfo(serviceName);
        const PROTO::Service* service = serviceINfo.GetService();
        const PROTO::MethodDescriptor *methodDes = serviceINfo.GetMethodDescriptor(methodName);

        //获取request请求指针
        PROTO::Message *req = service->GetRequestPrototype(methodDes).New();
        //解析request信息
        parseRequast(req, data, argsSize);

        // 获取response指针
        PROTO::Message *resp = service->GetResponsePrototype(methodDes).New();

        //将responseToClient转换为client响应消息类型
        std::function<void()> methodCallback(std::bind(RpcProvider::responseToClient, this, connectPtr, req));
        PROTO::Closure *closure = PROTO::NewCallback(methodCallback.target<void()>());
        
        // 调用protobuf抽象后的rpc方法
        const_cast<PROTO::Service*>(service)->CallMethod(methodDes, nullptr, req, resp, closure);
    }
    catch (except err)
    {
        LOG_ERROR << err.what();
    }
}

mprpcHeader::rpcHeader RpcProvider::parseHeadler(const std::string &data)
{
    //判断是否满足最小包条件大小
    if (data.size() <= sizeof(mprpcHeader::rpcHeader) + 4)
        throw except("数据包不满足最低大小");
    //获取rpcHeadler长度
    int headlerSize = std::atoi(data.substr(0, 4).c_str());
    mprpcHeader::rpcHeader headler;
    //解析handler的protobuf数据
    if (!headler.ParseFromString(data.substr(4, headlerSize))){
        throw except("protobuf headler协议解析失败");
    }
    return headler;
}

void RpcProvider::responseToClient(const TcpConnectionPtr& respondPtr, PROTO::Message *response)
{
    std::string output;
    if(!response->SerializeToString(&output)){
        throw except("protobuf序列化协议解析失败");
    }
    respondPtr->send(output);
}

void RpcProvider::parseRequast(PROTO::Message * req,const std::string &data, uint32_t argsSize)
{

    int headlerSize = std::atoi(data.substr(0, 4).c_str());
    if(data.size() != 4 + headlerSize + argsSize)
        throw except("数据包实际大小与理论大小不符合");
    // auto it = m_serviceMap.find(serviceName);
    // if(it == m_serviceMap.end())
    //     throw except("未在服务表中找到对应的服务类");
    //const PROTO::Message *req = it->second.m_service->GetRequestPrototype();
    if(!req->ParseFromString(data.substr(4 + headlerSize, argsSize)))
        throw except("protobuf request协议解析失败");
    return;
}

const PROTO::Service *RpcProvider::ServiceInfo::GetService() const
{
    return m_service;
}

const PROTO::MethodDescriptor *RpcProvider::ServiceInfo::GetMethodDescriptor(const std::string methodName) const
{
    auto it = m_methodMap.find(methodName);
    if (it == m_methodMap.end())
        throw except("未在方法描述表中找到对应的方法描述");
    return it->second;
}

void RpcProvider::ServiceInfo::SetService(PROTO::Service * service)
{
    m_service = service;
}

void RpcProvider::ServiceInfo::AddMethodDescripto(const std::string methodName, const PROTO::MethodDescriptor *methodDescriptor)
{
    m_methodMap.emplace(methodName, methodDescriptor);
}

const RpcProvider::ServiceInfo &RpcProvider::GetServiceInfo(std::string serviceName) const
{
    auto it = m_serviceMap.find(serviceName);
    if (it == m_serviceMap.end())
        throw except("未在服务表中找到对应的服务类");
    return it->second;
}
