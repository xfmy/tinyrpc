#include "rpcProvider.h"
#include "mprpcApplication.h"
#include "protobuf.pb.h"
#include "muduo/base/Logging.h"
#include "string"

class UserService : public fixbug::user
{
public:
    bool Login(std::string name, std::string password){
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
        done->Run();
    }
};

int main()
{
    MprpcApplication::GetInstance().init();
    RpcProvider provider;
    provider.NotifyService(new UserService());
    provider.run();
    return 0;
}