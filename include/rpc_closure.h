#pragma once

#include <google/protobuf/stubs/callback.h>
#include <functional>
#include <memory>

namespace tinyrpc {

class RpcClosure : public google::protobuf::Closure
{
public:
    typedef std::shared_ptr<RpcClosure> rpcClosurePtr;
    explicit RpcClosure(std::function<void()> cb) : cb_(cb) {}

    ~RpcClosure() = default;

    /// @brief 关闭回调函数
    void Run()
    {
        if (cb_)
        {
            cb_();
        }
    }

private:
    /// @brief 保存回调对象
    std::function<void()> cb_{nullptr};
};

} // namespace tinyrpc
