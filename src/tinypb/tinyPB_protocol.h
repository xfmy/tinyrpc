#pragma once
#include <cstdint>
#include <string>

namespace mprpc {
class TinyPBProtocol
{
public:
    TinyPBProtocol() = default;
    ~TinyPBProtocol() = default;

public:
    /// @brief 包头标识,固定为0xFE
    static char     PB_START;
    /// @brief 包尾标识,固定为0xFF
    static char     PB_END;

public:
    /// @brief 整包长度
    int32_t         packageLen_{0};
    
    /// @brief rpc消息唯一标识id长度
    int32_t         msgIdLen_{0};
    /// @brief rpc消息唯一标识
    std::string     msgId_;

    /// @brief rpc方法名长度
    int32_t         methodNameLen_{0};
    /// @brief rpc方法名
    std::string     methodName_;

    /// @brief 错误码
    int32_t         errorCode_{0};
    /// @brief 错误信息长度
    int32_t         errorInfoLen_{0};
    /// @brief 错误信息
    std::string     errorInfo_{0};

    /// @brief 实际rpc请求数据
    std::string     pbData_;
    /// @brief 校验码
    int32_t         checksum_{0};

    /// @brief 判断是否已经被解析成功
    bool            parseSuccess_{false};
};
} // namespace mprpc