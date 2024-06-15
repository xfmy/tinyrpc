#pragma once
/**
 @file except.hpp
 @mainpage 自定义异常包装类
 @brief 通过c++17 std::source_location类在抛出异常时获取局部信息
*/

#include <source_location>
#include <exception>
#include <string>

namespace tinyrpc {
class tinyrpcExcept final : std::exception
{
public:
    /// @brief 通过std::source_location::current()可以获取异常抛出点的位置信息
    /// @param errorMsg 用户自定义错误消息
    /// @param sourceInfo 记录抛出点的地址信息
    tinyrpcExcept(
        const std::string errorMsg, int32_t errorCode = 0,
        std::source_location sourceInfo = std::source_location::current())
    {
        errorInfo_ = errorInfo_ + sourceInfo.file_name() + ' ' +
                     sourceInfo.function_name() + ' ' +
                     std::to_string(sourceInfo.line()) + ' ' + errorMsg;
    }

    /// @brief 获取异常信息
    /// @return 返回异常信息字符串
    std::string what() { return errorInfo_; }

    /// @brief 获取错误码
    /// @return 返回错误码
    int GetErrorCode() { return errorCode_; }

private:
    std::string errorInfo_; ///< 错误信息
    int errorCode_{0};      ///< 错误码
};
} // namespace tinyrpc