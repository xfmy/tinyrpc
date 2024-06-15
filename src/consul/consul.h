/**
 * @file consul.h
 * @brief consul服务注册,发现,取消注册,心跳,健康检查
 * 
 */
#pragma once
#include <string>
#include <memory>
#include <ppconsul/ppconsul.h>
#include <ppconsul/agent.h>
#include <ppconsul/health.h>
#include <muduo/net/InetAddress.h>
#include "rpc_application.h"

namespace tinyrpc {
class ConsulClient
{
public:
    ConsulClient();

    /**
     * @brief 注册consul服务 
     * 
     * @param name 
     * @param id 
     * @param address 
     * @param port 
     */
    void RegisterService(const std::string& name, const std::string& id,
                         const std::string& address, uint16_t port);

    /**
     * @brief 取消注册
     * 
     * @param id 
     */
    void DeregisterService(const std::string& id);

    /**
     * @brief 服务发现
     * 
     * @param name 服务名
     * @return 返回服务发现地址信息 
     */
    std::unique_ptr<muduo::net::InetAddress> DiscoverService(
        const std::string& name);
    /**
     * @brief consul服务心跳
     * 
     * @param serviceId 服务id
     */
    void ServicePass(std::string serviceId);

private:
    /**
     * @brief 服务健康检测
     * 
     * @param service 查询服务节点信息
     * @return true 健康
     * @return false 不健康
     */
    bool CheckHealth(ppconsul::health::NodeServiceChecks service);

private:
    /// @brief ppconsul操作指针
    std::shared_ptr<ppconsul::Consul> consulPtr_;
};
} // namespace tinyrpc