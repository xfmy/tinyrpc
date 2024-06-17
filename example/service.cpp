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
        // 框架给业务上报了请求参数LoginRequest，应用获取相应数据做本地业务
        std::string name = requst->name();
        std::string password = requst->pwd();

        // 做本地业务
        int res = Login(name, password);

        // 把响应写入，包括错误码、错误消息、返回值
        fixbug::ResultCode* resCode = response->mutable_result();
        resCode->set_errcode(0);
        resCode->set_errmsg("一切正常 very good");
        response->set_sucess(res);
        // 执行回调操作,
        // 执行响应对象数据的序列化和网络发送（都是由框架来完成的）
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