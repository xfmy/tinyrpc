#include "rpcProvider.h"
#include "mprpcApplication.h"
#include "mprpcNetwork.h"
#include "protobuf.pb.h"
#include <google/protobuf/descriptor.h>
#include <muduo/base/Logging.h>

    RpcProvider::RpcProvider()
{

}

void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    ServiceInfo serviceINfo;

    //获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor* pserviceDesc = service->GetDescriptor();
    //获取服务的名字
    std::string serviceName = pserviceDesc->name();
    //获取服务对象的service方法数量
    int methodCount = pserviceDesc->method_count();
    for (size_t i = 0; i < methodCount; i++)
    {
        //获取下标相应的方法描述
        const google::protobuf::MethodDescriptor* pmethodDesc = pserviceDesc->method(i);
        //h获取函数名
        std::string functionName = pmethodDesc->name();
        //将函数名与方法描述加入函数表中
        serviceINfo.m_methodMap.emplace(functionName, pmethodDesc);
    }
    //添加服务表
    serviceINfo.m_service = service;
    m_serviceMap.emplace(serviceINfo);

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

void RpcProvider::serviceDistribute(const TcpConnectionPtr& , const std::string& data, Timestamp)
{
    
}
