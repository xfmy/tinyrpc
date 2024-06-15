/**
 * @file service.cpp
 * @brief rpc server示例代码
 *
 */
#include <string>
#include <muduo/base/Logging.h>

#include "rpc_dispatcher.h"
#include "rpc_application.h"
#include "protobuf.pb.h"

using namespace tinyrpc;

//继承基类,实现Login虚方法
class UserService : public fixbug::user
{
public:
    bool Login(std::string name, std::string password)
    {
        LOG_INFO << "user name is " + name;
        LOG_INFO << "user password is " + password;
        return true;
    }

    virtual void Login(google::protobuf::RpcController* controller,
                       const fixbug::LoginRequest* requst,
                       fixbug::LoginResponse* response,
                       google::protobuf::Closure* done) override
    {
        /*
            或取请求参数
            调用本地对应的方法
            通过请求参数获取本地对应的信息
            封装响应消息，通过回到用返回给rpcClient端
        */
        std::string name = requst->name();
        std::string password = requst->pwd();
        int res = Login(name, password);
        fixbug::ResultCode* resCode = response->mutable_result();
        resCode->set_errcode(0);
        resCode->set_errmsg("一切正常 very good");
        response->set_sucess(res);
        // Closure是一个抽象类，
        done->Run();
    }
};

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cout << "启动项参数异常,请检查: ./server initConfigFile.conf"
                  << std::endl;
        return -1;
    }
    // 通过配置文件初始化框架
    RpcApplication::GetInstance().init(argv[1]);

    // 注册rpc服务
    RpcDispatcher provider;
    std::shared_ptr<UserService> userService = std::make_shared<UserService>();
    provider.registerService(userService);
    // 启动rpc
    provider.run();
    return 0;
}