/**
 * @file rpc_init_config.h
 * @mainpage tinyrpc框架的基础类,负责框架的初始化操作
 * @brief 初始化配置全局信息
 */

#pragma once
#include <boost/noncopyable.hpp>
#include "rpc_init_config.h"
#include <optional>

namespace tinyrpc {
/// @brief tinyrpc框架的基础类,负责框架的初始化操作
class RpcApplication : public boost::noncopyable
{
public:
    /// @brief 单例模式,获取单例对象
    /// @return 返回单例对象
    static RpcApplication &GetInstance(void)
    {
        static RpcApplication obj;
        return obj;
    }

    /// @brief 初始化配置项
    void init(std::string path);

    /// @brief 获取相应的配置值
    /// @param field key
    /// @return 如果key存在,返回std::option包装的string
    std::optional<std::string> atConfigItem(std::string field) const;

private:
    /// @brief 配置信息键值对
    configInfo infoMap;
};
} // namespace tinyrpc