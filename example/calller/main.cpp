#include "rpcChannel.h"
#include "mprpcApplication.h"
#include "protobuf.pb.h"

int main()
{
    //初始化框架
    MprpcApplication::GetInstance().init();

    //演示调用远程发布的rpc方法Login
    fixbug::user_Stub stub(new RpcChannel());

    //rpc参数
    fixbug::LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");

    //rpc的响应
    fixbug::LoginResponse response;

    //发起rpc方法的调用
    stub.Login(nullptr, &request, &response, nullptr);

    if(0 == response.result().errcode()){
        std::cout << "rpc call success:" << response.sucess() << std::endl;
    }
    else
    {
        std::cout << "rpc call error:" << response.result().errmsg() << std::endl;
    }

    return 0;
}
