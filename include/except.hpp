#pragma once

/**
 @file except.hpp
 @mainpage 自定义异常包装类
 @brief 通过c++17 std::source_location类在抛出异常时获取局部信息
*/
#include <source_location>
#include <exception>
#include <string>

namespace mprpc
{
class MPRpcExcept final : std::exception
{
private:
    std::string errorInfo_;
    int errorCode_{0};
public:
    /// @brief 通过std::source_location::current()可以获取异常抛出点的位置信息
    /// @param errorMsg 用户自定义错误消息
    /// @param sourceInfo 记录抛出点的地址信息
    MPRpcExcept(const std::string errorMsg, int32_t errorCode = 0,std::source_location sourceInfo =
                                                std::source_location::current())
    {
        errorInfo_ = errorInfo_ + sourceInfo.file_name() + ' ' + sourceInfo.function_name() + ' ' + std::to_string(sourceInfo.line()) + ' ' + errorMsg;
    }

    /// @brief 获取异常信息
    /// @return 返回异常信息字符串
    std::string what()
    {
        return errorInfo_;
    }

    int GetErrorCode()
    {
        return errorCode_;
    }
};
}