#pragma once
#include <string>
#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>
#include <muduo/net/InetAddress.h>

namespace tinyrpc
{
/**
 * @brief 控制类 
 * 
 */
class RpcController : public google::protobuf::RpcController
{
public:
    RpcController() = default;
    ~RpcController() = default;

    void Reset() override;

    bool Failed() const override;

    std::string ErrorText() const override;

    void StartCancel() override;

    void SetFailed(const std::string& reason) override;

    bool IsCanceled() const override;

    void NotifyOnCancel(google::protobuf::Closure* callback) override;
    /**************************************************************************************************/
    void SetError(int32_t error_code, const std::string error_info);

    int32_t GetErrorCode();

    std::string GetErrorInfo();

    void SetMsgId(const std::string& msg_id);

    std::string GetMsgId();

    void SetLocalAddr(muduo::net::InetAddress addr);

    void SetPeerAddr(muduo::net::InetAddress addr);

    muduo::net::InetAddress GetLocalAddr();

    muduo::net::InetAddress GetPeerAddr();

    void SetTimeout(int timeout);

    int GetTimeout();

    bool Finished();

    void SetFinished(bool value);

private:
    /// @brief 错误码
    int32_t errorCode_{0};
    /// @brief 错误信息
    std::string errorInfo_;
    //消息类型
    std::string msgId_;

    /// @brief rpc调用是否出现错误
    bool isFailed_{false};
    /// @brief rpc调用是否已经取消
    bool isCancled_{false};
    /// @brief rpc调用是否完成
    bool isFinished_{false};

    /// @brief 存储本地地址
    muduo::net::InetAddress localAddr_;
    /// @brief 存储对端地址
    muduo::net::InetAddress peerAddr_;

    /// @brief rpc调用默认超时时间
    int timeout_{1000}; //单位: ms
};
}