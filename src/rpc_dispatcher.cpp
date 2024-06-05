#include <google/protobuf/descriptor.h>

#include <google/protobuf/message.h>
#include <muduo/base/Logging.h>

#include "rpc_dispatcher.h"
#include "rpc_application.h"
#include "tcp_server.h"
#include "rpc_controller.h"
#include "rpc_closure.h"
#include "tinyPB_protocol.h"
#include "tinyPB_coder.h"
#include "except.hpp"
#include "error_code.h"
#include "consul.h"
#include "random_number.h"

namespace tinyrpc {
RpcDispatcher::RpcDispatcher()
{
    consulPtr_ = std::make_shared<ConsulClient>();
}

RpcDispatcher::~RpcDispatcher()
{
    for (int i = 0; i < services_.size(); i++)
        consulPtr_->DeregisterService(services_[i]);
}

void RpcDispatcher::registerService(servicePtr service)
{
    std::string serviceName = service->GetDescriptor()->name();
    serviceMap_[serviceName] = service;

    std::string serviceIp =
        RpcApplication::GetInstance().atConfigItem("servicePublicIp").value();
    std::string servicePort =
        RpcApplication::GetInstance().atConfigItem("servicePort").value();
    //std::string serviceName = service->GetDescriptor()->name();
    std::string ID = serviceName + '-' + tinyrpc::GetRandomNumberString();

    consulPtr_->RegisterService(serviceName, ID, serviceIp,
                                std::atoi(servicePort.c_str()));
    services_.push_back(ID);
}

void RpcDispatcher::run()
{
    std::optional<std::string> servicePort =
        RpcApplication::GetInstance().atConfigItem("servicePort");
    if (servicePort.has_value())
    {
        int port = std::atoi(servicePort.value().c_str());
        TcpServer sessionLayer(port);
        //设置rpc分发回调函数
        sessionLayer.setDispatchCallback(
            std::bind(&RpcDispatcher::dispatch, this, _1, _2, _3));

        //向consul注册服务回调
        std::function<void()> cb = [this]() -> void {
            for (std::string serviceId : services_)
            {
                consulPtr_->ServicePass(serviceId);
            }
        };
        cb();
        sessionLayer.AddTimerEvent(30, cb);
        sessionLayer.start();
    }
    else
    {
        LOG_FATAL << "serverPort not config";
        exit(-1);
    }
}

void RpcDispatcher::writeComplete(
    const muduo::net::TcpConnectionPtr &connectPtr)
{
    // TODO 短链接?
    connectPtr->shutdown();
}

void RpcDispatcher::dispatch(std::shared_ptr<TinyPBProtocol> request,
                             std::shared_ptr<TinyPBProtocol> response,
                             const muduo::net::TcpConnectionPtr &ptr)
{
    try
    {
        std::string methonFullName = request->methodName_;
        std::string serviceName;
        std::string methodName;

        if (!parseServiceFullName(methonFullName, serviceName, methodName))
        {
            setTinyPBError(response, ERROR_PARSE_SERVICE_NAME,
                           "parse service name error");
            return;
        }

        response->msgId_ = request->msgId_;
        response->methodName_ = request->methodName_;

        auto it = serviceMap_.find(serviceName);
        if (it == serviceMap_.end())
        {
            LOG_ERROR << request->msgId_ + "| sericve neame[" + serviceName +
                             "] not found";
            setTinyPBError(response, ERROR_SERVICE_NOT_FOUND,
                           "service not found");
            return;
        }

        servicePtr service = (*it).second;

        const PROTO::MethodDescriptor *method =
            service->GetDescriptor()->FindMethodByName(methodName);
        if (method == NULL)
        {
            LOG_ERROR << request->msgId_ + "method neame[" + methodName +
                             "] not found in service[" + serviceName;
            setTinyPBError(response, ERROR_SERVICE_NOT_FOUND,
                           "method not found");
            return;
        }

        PROTO::Message *req_msg = service->GetRequestPrototype(method).New();

        // 反序列化，将 pb_data 反序列化为 req_msg
        if (!req_msg->ParseFromString(request->pbData_))
        {
            LOG_ERROR << request->msgId_ + "deserilize error" + methodName +
                             serviceName;
            setTinyPBError(response, ERROR_FAILED_DESERIALIZE,
                           "deserilize error");
            return;
        }

        LOG_INFO << request->msgId_ + "| get rpc request" +
                        req_msg->ShortDebugString();
        PROTO::Message *resp_msg = service->GetResponsePrototype(method).New();

        RpcController *rpcController = new RpcController();
        rpcController->SetLocalAddr(ptr->localAddress());
        rpcController->SetPeerAddr(ptr->peerAddress());
        rpcController->SetMsgId(request->msgId_);

        RpcClosure *closure = new RpcClosure([req_msg, resp_msg, request,
                                              response, ptr, rpcController,
                                              this]() mutable {
            if (!resp_msg->SerializeToString(&(response->pbData_)))
            {
                LOG_ERROR << std::string() + request->msgId_ +
                                 "| serilize error, origin message ->" +
                                 resp_msg->ShortDebugString();
                setTinyPBError(response, ERROR_FAILED_SERIALIZE,
                               "serilize error");
            }
            else
            {
                response->errorCode_ = 0;
                response->errorInfo_ = "";
                LOG_INFO << std::string() + request->msgId_ +
                                "| dispatch success, requesut->" +
                                req_msg->ShortDebugString() + ", response->" +
                                resp_msg->ShortDebugString();
            }

            // std::vector<AbstractProtocol::s_ptr> replay_messages;
            // replay_messages.emplace_back(rsp_protocol);
            //将rsp_protocol发送client
            // ptr->reply(replay_messages);

            // resp_msg->SerializeToString(&response->pbData_);
            responseToClient(ptr, response);
        });

        service->CallMethod(method, rpcController, req_msg, resp_msg, closure);
    }
    catch (tinyrpcExcept err)
    {
        LOG_ERROR << err.what();
    }
    catch (std::exception err)
    {
        LOG_ERROR << err.what();
    }
}

void RpcDispatcher::responseToClient(const TcpConnectionPtr &ptr,
                                     std::shared_ptr<TinyPBProtocol> response)
{
    std::string output;
    tinyrpc::TinyPBCoder::encode(response, output);

    //     if (!response->SerializeToString(&output))
    // {
    //     throw tinyrpcExcept("protobuf序列化协议解析失败");
    // }
    ptr->send(output);
    // respondPtr->shutdown();
}

bool RpcDispatcher::parseServiceFullName(const std::string &full_name,
                                         std::string &service_name,
                                         std::string &method_name)
{
    //"fixbug.user.Login"
    //"user.Login"
    if (full_name.empty())
    {
        LOG_ERROR << "full name empty";
        return false;
    }
    // size_t i = full_name.find_first_of(".");

    size_t i = full_name.find_last_of(".");

    if (i == full_name.npos)
    {
        LOG_ERROR << "not find . in full name " + full_name;
        return false;
    }
    // service_name = full_name.substr(0, i);
    // method_name = full_name.substr(i + 1, full_name.length() - i - 1);

    method_name = full_name.substr(i + 1, full_name.length() - i - 1);
    service_name = full_name.substr(0, i);
    if (size_t temp = service_name.find("."); temp != full_name.npos)
    {
        service_name =
            service_name.substr(temp + 1, service_name.size() - temp);
    }

    LOG_ERROR << "parse sericve_name[" + service_name + "] and method_name[" +
                     method_name + "] from full name [" + full_name + "]";

    return true;
}
void RpcDispatcher::setTinyPBError(std::shared_ptr<TinyPBProtocol> msg,
                                   int32_t err_code, const std::string err_info)
{
    msg->errorCode_ = err_code;
    msg->errorInfo_ = err_info;
    msg->errorInfoLen_ = err_info.size();
}
} // namespace tinyrpc

// tinyrpcHeader::rpcHeader RpcDispatcher::parseHeadler(const std::string &data)
// {
//     //判断是否满足最小包条件大小
//     // if (data.size() <= sizeof(tinyrpcHeader::rpcHeader) + 4)
//     //     throw except("数据包不满足最低大小");
//     //获取rpcHeadler长度

//     int headlerSize = *(int *)data.c_str();
//     tinyrpcHeader::rpcHeader headler;
//     //解析handler的protobuf数据
//     if (!headler.ParseFromString(data.substr(4, headlerSize)))
//     {
//         throw except("protobuf headler协议解析失败");
//     }
//     return headler;
// }

// void RpcDispatcher::serviceDistribute(
//     const muduo::net::TcpConnectionPtr &connectPtr, const std::string &data,
//     muduo::Timestamp)
// {
//     try
//     {
//         //解析rpcHeadler头部并获取相应的字段
//         tinyrpcHeader::rpcHeader headler = parseHeadler(data);
//         std::string serviceName = headler.servicename();
//         std::string methodName = headler.methodname();
//         uint32_t argsSize = headler.argssize();

//         // 获取service信息
//         const ServiceInfo &serviceINfo = GetServiceInfo(serviceName);
//         const PROTO::Service *service = serviceINfo.GetService();
//         const PROTO::MethodDescriptor *methodDes =
//             serviceINfo.GetMethodDescriptor(methodName);

//         //获取request请求指针
//         PROTO::Message *req = service->GetRequestPrototype(methodDes).New();
//         //解析request信息
//         parseRequast(req, data, argsSize);

//         // 获取response指针
//         PROTO::Message *resp =
//         service->GetResponsePrototype(methodDes).New();

//         //将responseToClient转换为client响应消息类型
//         // std::function<void()>
//         // methodCallback(std::bind(RpcDispatcher::responseToClient, this,
//         // connectPtr, req)); PROTO::Closure *closure =
//         // PROTO::NewCallback(methodCallback.target<void()>());
//         PROTO::Closure *closure =
//             PROTO::NewCallback<RpcDispatcher, const TcpConnectionPtr &,
//                                PROTO::Message *>(
//                 this, &RpcDispatcher::responseToClient, connectPtr, resp);

//         // 调用protobuf抽象后的rpc方法
//         const_cast<PROTO::Service *>(service)->CallMethod(methodDes, nullptr,
//                                                           req, resp,
//                                                           closure);
//     }
//     catch (tinyrpcExcept err)
//     {
//         LOG_ERROR << err.what();
//     }
// }

// const RpcDispatcher::ServiceInfo &RpcDispatcher::GetServiceInfo(
//     std::string serviceName) const
// {
//     auto it = m_serviceMap.find(serviceName);
//     if (it == m_serviceMap.end()) throw
//     except("未在服务表中找到对应的服务类"); return it->second;
// }

// void RpcDispatcher::ServiceInfo::SetService(PROTO::Service *service)
// {
//     m_service = service;
// }

// const PROTO::MethodDescriptor
// *RpcDispatcher::ServiceInfo::GetMethodDescriptor(
//     const std::string methodName) const
// {
//     auto it = m_methodMap.find(methodName);
//     if (it == m_methodMap.end())
//         throw except("未在方法描述表中找到对应的方法描述");
//     return it->second;
// }

// void RpcDispatcher::ServiceInfo::AddMethodDescripto(
//     const std::string methodName,
//     const PROTO::MethodDescriptor *methodDescriptor)
// {
//     m_methodMap.emplace(methodName, methodDescriptor);
// }

// const PROTO::Service *RpcDispatcher::ServiceInfo::GetService() const
// {
//     return m_service;
// }

// void RpcDispatcher::parseRequast(PROTO::Message *req, const std::string
// &data,
//                                  uint32_t argsSize)
// {
//     int headlerSize = *(int *)data.c_str();
//     if (data.size() != 4 + headlerSize + argsSize)
//         throw except("数据包实际大小与理论大小不符合");
//     // auto it = m_serviceMap.find(serviceName);
//     // if(it == m_serviceMap.end())
//     //     throw except("未在服务表中找到对应的服务类");
//     // const PROTO::Message *req =
//     it->second.m_service->GetRequestPrototype(); if
//     (!req->ParseFromString(data.substr(4 + headlerSize, argsSize)))
//         throw except("protobuf request协议解析失败");
//     return;
// }