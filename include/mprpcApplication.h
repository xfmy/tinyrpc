#pragma once
#include <boost/noncopyable.hpp>
#include "rpcInitConfig.h"
#include <optional>
/**
 * @file mprpcApplication.h
 * @mainpage mprpc框架的基础类,负责框架的初始化操作
 * @brief 初始化配置全局信息
 */


/// @brief mprpc框架的基础类,负责框架的初始化操作
class MprpcApplication : public boost::noncopyable{
public:
    /// @brief 单例模式,获取单例对象
    /// @return 返回单例对象
    static MprpcApplication &GetInstance(void)
    {
        static MprpcApplication obj;
        return obj;
    }

    /// @brief 初始化配置项
    void init();

    /// @brief 获取相应的配置值
    /// @param field key
    /// @return 如果key存在,返回std::option包装的string
    std::optional<std::string> atConfigItem(std::string field) const;

private:
    configInfo infoMap;
};