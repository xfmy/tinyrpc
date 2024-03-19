#pragma once

#include <string>
#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>
#include <muduo/net/InetAddress.h>

namespace mprpc
{
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
    int32_t errorCode_{0};
    std::string errorInfo_;
    std::string msgId_;

    bool isFailed_{false};
    bool isCancled_{false};
    bool isFinished_{false};

    muduo::net::InetAddress localAddr_;
    muduo::net::InetAddress peerAddr_;

    int timeout_{1000}; // ms
};
}