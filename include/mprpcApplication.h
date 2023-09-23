#pragma once
#include <boost/noncopyable.hpp>
#include "rpcInitConfig.h"
#include <optional>

//mprpc框架的基础类,负责框架的初始化操作
class MprpcApplication : public boost::noncopyable{
public:

    //单例模式
    static const MprpcApplication &GetInstance(void)
    {
        static MprpcApplication obj;
        return obj;
    }

    //初始化配置项
    void init() const;

    //获取相应的配置值
    std::optional<std::string> atConfigItem(std::string field) const;

private:
    configInfo infoMap;
};