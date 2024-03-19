#include "rpc_controller.h"

namespace mprpc {

void RpcController::Reset()
{
    errorCode_ = 0;
    errorInfo_ = "";
    msgId_ = "";
    isFailed_ = false;
    isCancled_ = false;
    isFinished_ = false;
    //localAddr_ = nullptr;
    //peerAddr_ = nullptr;
    timeout_ = 1000; // ms
}

bool RpcController::Failed() const { return isFailed_; }

std::string RpcController::ErrorText() const { return errorInfo_; }

void RpcController::StartCancel()
{
    isCancled_ = true;
    isFailed_ = true;
    SetFinished(true);
}

void RpcController::SetFailed(const std::string& reason)
{
    errorInfo_ = reason;
    isFailed_ = true;
}

bool RpcController::IsCanceled() const { return isCancled_; }

void RpcController::NotifyOnCancel(google::protobuf::Closure* callback) {}

void RpcController::SetError(int32_t error_code, const std::string error_info)
{
    errorCode_ = error_code;
    errorInfo_ = error_info;
    isFailed_ = true;
}

int32_t RpcController::GetErrorCode() { return errorCode_; }

std::string RpcController::GetErrorInfo() { return errorInfo_; }

void RpcController::SetMsgId(const std::string& msg_id) { msgId_ = msg_id; }

std::string RpcController::GetMsgId() { return msgId_; }

void RpcController::SetLocalAddr(muduo::net::InetAddress addr)
{
    localAddr_ = addr;
}

void RpcController::SetPeerAddr(muduo::net::InetAddress addr)
{
    peerAddr_ = addr;
}

muduo::net::InetAddress RpcController::GetLocalAddr() { return localAddr_; }

muduo::net::InetAddress RpcController::GetPeerAddr() { return peerAddr_; }

void RpcController::SetTimeout(int timeout) { timeout_ = timeout; }

int RpcController::GetTimeout() { return timeout_; }

bool RpcController::Finished() { return isFinished_; }

void RpcController::SetFinished(bool value) { isFinished_ = value; }

} // namespace mprpc