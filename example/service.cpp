#include "rpc_dispatcher.h"
#include "rpc_application.h"
#include "protobuf.pb.h"
#include "muduo/base/Logging.h"
#include "string"
using namespace tinyrpc;
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
        std::cout << "启动项参数异常,请检查" << std::endl;
        return 1;
    }
    RpcApplication::GetInstance().init(argv[1]);
    RpcDispatcher provider;
    std::shared_ptr<UserService> userService = std::make_shared<UserService>();
    provider.registerService(userService);
    provider.run();
    return 0;
}