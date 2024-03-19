#pragma once

#include <google/protobuf/stubs/callback.h>
#include <functional>
#include <memory>
//#include "rocket/common/run_time.h"
//#include "rocket/common/log.h"
//#include "except.hpp"
//#include "rpc_interface.h"
//#include <muduo/base/Logging.h>
//using namespace muduo::Logger;

namespace mprpc {

class RpcClosure : public google::protobuf::Closure
{
public:
    typedef std::shared_ptr<RpcClosure> rpcClosurePtr;
    explicit RpcClosure(std::function<void()> cb) : cb_(cb) {}

    ~RpcClosure() = default;

    void Run()
    {
        if (cb_)
        {
            cb_();
        }
    }

private:
    std::function<void()> cb_{nullptr};
};

} // namespace mprpc

/*
namespace mprpc
{
    class RpcClosure : public google::protobuf::Closure
    {
    public:
        typedef std::shared_ptr<RpcInterface> it_s_ptr;

        RpcClosure(it_s_ptr interface, std::function<void()> cb)
            : m_rpc_interface(interface),
              m_cb(cb)
        {
            LOG_INFO << "RpcClosure";
        }

        ~RpcClosure() { LOG_INFO << "~RpcClosure"; }

        //执行Closure设置的回调
        void Run() override
        {
            // 更新 runtime 的 RpcInterFace, 这里在执行 cb 的时候，都会以
            // RpcInterface 找到对应的接口，实现打印 app 日志等
            if (!m_rpc_interface)
            {
                //TODO 处理
                //RunTime::GetRunTime()->m_rpc_interface = m_rpc_interface.get();
            }

            try
            {
                if (m_cb != nullptr)
                {
                    m_cb();
                }
                if (m_rpc_interface)
                {
                    m_rpc_interface.reset();
                }
            }
            catch (MPRpcExcept& e)
            {
                LOG_ERROR << "MPRpcExcept exception[" + e.what() + "], deal handle";
                //e.handle();
                if (m_rpc_interface)
                {
                    m_rpc_interface->setError(e.GetErrorCode(), e.what());
                    m_rpc_interface.reset();
                }
            }
            catch (std::exception& e)
            {
                LOG_ERROR << e.what();
                if (m_rpc_interface)
                {
                    m_rpc_interface->setError(-1, "unkonwn std::exception");
                    m_rpc_interface.reset();
                }
            }
            catch (...)
            {
                LOG_ERROR << "Unkonwn exception";
                if (m_rpc_interface)
                {
                    m_rpc_interface->setError(-1, "unkonwn exception");
                    m_rpc_interface.reset();
                }
            }
        }

    private:
        it_s_ptr m_rpc_interface{nullptr};
        std::function<void()> m_cb{nullptr};
    };
}
*/