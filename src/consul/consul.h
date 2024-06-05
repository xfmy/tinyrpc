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

    void DeregisterService(const std::string& id);

    std::unique_ptr<muduo::net::InetAddress> DiscoverService(
        const std::string& name);
    /**
     * @brief 
     * 
     * @param serviceId 
     */
    void ServicePass(std::string serviceId);

private:

    bool CheckHealth(ppconsul::health::NodeServiceChecks service);

private:
    std::shared_ptr<ppconsul::Consul> consulPtr_;
};
} // namespace tinyrpc