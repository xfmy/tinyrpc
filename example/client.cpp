/**
 * @file client.cpp
 * @brief rpc client 示例代码
 * 
 */

#include "rpc_channel.h"
#include "rpc_controller.h"
#include "rpc_closure.h"
#include "rpc_application.h"
#include "protobuf.pb.h"
#include "muduo/net/InetAddress.h"
using namespace tinyrpc;
int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cout << "启动项参数异常,请检查: ./client initConfigFile.conf"
                  << std::endl;
        return -1;
    }
    //初始化框架
    RpcApplication::GetInstance().init(argv[1]);

    //演示调用远程发布的rpc方法Login
    fixbug::user_Stub stub(new RpcChannel());

    // rpc参数
    fixbug::LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");

    // rpc的响应
    fixbug::LoginResponse response;

    //发起rpc方法的调用
    RpcController control;

    stub.Login(&control, &request, &response, nullptr);

    //一次rpc调用完成，读调用的结果
    //不要直接访问response
    if (control.Failed())
    {
        std::cout << control.ErrorText() << std::endl;
    }
    else
    {
        if (0 == response.result().errcode())
        {
            std::cout << "rpc call success:" << response.sucess()
                      << response.result().errcode()
                      << response.result().errmsg() << std::endl;
        }
        else
        {
            std::cout << "rpc call error:" << response.result().errmsg()
                      << std::endl;
        }
    }

    return 0;
}
