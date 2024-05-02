#include "rpc_channel.h"
#include "rpc_controller.h"
#include "rpc_closure.h"
#include "rpc_application.h"
#include "protobuf.pb.h"
#include "muduo/net/InetAddress.h"
using namespace mprpc;
int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cout << "启动项参数异常,请检查" << std::endl;
        return 1;
    }
    //初始化框架
    RpcApplication::GetInstance().init(argv[1]);

    //演示调用远程发布的rpc方法Login
    // muduo::net::InetAddress()
    fixbug::user_Stub stub(
        new RpcChannel(/*muduo::net::InetAddress("127.0.0.1", 9001)*/));

    // rpc参数
    fixbug::LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");

    // rpc的响应
    fixbug::LoginResponse response;

    //发起rpc方法的调用
    RpcController control;

    RpcClosure close([request, response, &control]() mutable {
        if (control.GetErrorCode() == 0)
        {
            // 执行业务逻辑
            std::cout << "RpcClosure callback" << std::endl;
        }
        else
        {
            
        }
    });

    stub.Login(&control, &request, &response, &close);

    if (0 == response.result().errcode())
    {
        std::cout << "rpc call success:" << response.sucess()
                  << response.result().errcode() << response.result().errmsg()
                  << std::endl;
    }
    else
    {
        std::cout << "rpc call error:" << response.result().errmsg()
                  << std::endl;
    }

    return 0;
}
