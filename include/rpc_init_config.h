/**
 * @file rpc_init_config.h
 * @brief 通过配置文件初始化rpc框架
 *
 */

#pragma once
#include <map>
#include <string>

using configInfo = std::map<std::string, std::string>;

namespace tinyrpc {
/// @brief 配置文件解析类
class RpcInitConfig
{
public:
    /// @brief 读取并解析配置文件
    /// @return 返回解析后的key-value
    static configInfo execute(std::string path);
};
} // namespace tinyrpc