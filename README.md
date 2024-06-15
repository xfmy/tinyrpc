# Tinyrpc

## 项目说明

该项目是在 Linux 环境下基于 muduo、Protobuf 和 Consul 实现的一个轻量级 分布式网络通信RPC 框架。可以把单体架构系统的本地方法调用，重构成基于 TCP 网络通信的 RPC 远程方法调用，实现统一台机器不同进程或者不同机器之间的服务调用。

## 项目特点

1. 基于 muduo 网络库实现高并发网络通信模块，作为 RPC 同步调用的基础。
2. 在通讯协议上，采用protobuf实现数据的序列化和反序列化。
3. 设计基于Tcp传输的二进制协议`tinypb`,解决粘包问题,且能够高效传输服务名、方法名以及参数。并且通过设置消息包id字段防止串包并方便日志追踪,并在不侵入业务代码的情况下可以设置错误码以及错误消息
4. 基于 Consul 分布式协调服务中间件提供服务注册和服务发现功能。
5. 通过`RpcController`实现客户端的超时机制。
6. 使用线程池做业务处理

## 项目环境配置

1.  安装`muduo`网络库
2.  安装序列化`protobuf`
3.  安装配置文件库`libconfig`
4.  安装格式化库`libfmt`
5.  安装consul库`ppconsul`以及依赖库`json11`,`curl`
6.  安装`cmake`以及`gcc`
7.  安装服务注册与发现中间件`consul`

## 开发环境

+ ubuntu 22.04
+ vscode远程
+ CMake构建项目集成编译环境
+ Gdb调试
+ Git版本管理

## 项目代码工程目录

- bin：可执行文件
- build：项目编译文件
- conf: 配置文件目录
- doc: 文档目录
- lib：项目库文件
- src：源文件
- test：测试代码
- example：框架代码使用范例
- CMakeLists.txt：顶层的cmake文件
- README.md：项目自述文件
- build.sh：一键编译脚本
- .clang-format: clang格式化文件
- .gitignore: git过滤文件

## 构建项目

运行脚本，其会自动编译项目

```bash
sh autobuild.sh 
```

最后会在`lib`目录下生成`libtinyrpc.a`rpc静态库文件,以及在`bin`目录下生成client,service示例程序

## 使用示例

#### 1.启动consul

将consul以及服务端的ip,port信息改写入配置文件conf/initConfigFile.conf

```
config = {
    #服务端配置信息
    servicePublicIp = "127.0.0.1";
    servicePort = "9001";

    #consul配置信息
    consulIp = "127.0.0.1";
    consulPort = "8500";
}
```

#### 2.运行示例程序

进入bin目录之下

```shell
#启动服务端
./service ../conf/initConfigFile.conf
#启动客户端
./client ../conf/initConfigFile.conf
```

### 如何使用

首先编写`protobuf`相关代码,如下示例

```protobuf
syntax = "proto3";//声明了protobuf的版本

package  fixbug;//声明代码所在位置,对于c++来说是namespace

option cc_generic_services = true;

//定义登录请求基本类型 name pwd
message LoginRequest
{
	string name = 1;//数字表示第几个参数
	string pwd = 2;
}

message ResultCode
{
	int32 errCode = 1;
	string errMsg = 2;
}

message LoginResponse
{
	ResultCode result = 1;
	bool sucess  = 2;
}

service user
{
    rpc Login(LoginRequest) returns(LoginResponse);
}
```

执行命令 protoc  protobuf.proto --cpp_out=OUT_DIR        将会在OUT_DIR目录下生成定义的test.proto文件的cpp代码

![image-20240502142445631](https://s2.loli.net/2024/05/02/RsGYP1e9q7Bt86x.png)

#### 服务端示例

首先你需要创建个类继承自利用protobuf生成的h文件，然后定义本地对应函数的实现

最后重写基类中的虚函数

在虚函数中主要做几件事情

1. 或取请求参数
2. 调用本地对应的方法
3. 通过请求参数获取本地对应的信息
4. 封装响应消息，通过回到用返回给rpcClient端

```cpp
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
        //Closure是一个抽象类，
        done->Run();
    }
};
```

然后通过框架启动一个rpc服务

```cpp
RpcApplication::GetInstance().init(argv[1]);
RpcDispatcher provider;
std::shared_ptr<UserService> userService = std::make_shared<UserService>();
//将你需要的服务可以注册到框架上
provider.registerService(userService);
provider.run();
```

#### 调用者示例

proto文件会自动生成服务端需要的类，也生成了客户端需要的类

调用者需要先调用框架的init方法接受参数，看需要调用哪个rpc服务

然后创建对应Rpc方法提供的stub类的类对象传入框架提供的channel方法，

闯将请求和响应对象，将请求时需要的参数通过protobuf提供的set方法写入到对象中

然后调用stub类对象的相应的方法，响应会通过respons参数传回来

```cpp
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
    fixbug::user_Stub stub( new RpcChannel());

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

```

调用一次打印日志

```
#service
root@VM-16-3-ubuntu:~/projects/mprpc/bin# ./service ../conf/initConfigFile.conf
20250331 14:44:06.634708 388175 INFO  work loop thread start - tcp_server.cpp:97
20250331 14:44:10.840627 388170 INFO  TcpServer::newConnection [networkServer] - new connection [networkServer-0.0.0.0:9001#1] from 127.0.0.1:58676 - TcpServer.cc:80
20250331 14:44:10.840823 388175 INFO  127.0.0.1:58676客户发起来了连接 - tcp_server.cpp:85
20250331 14:44:10.841466 388175 INFO  接收到一个的包 data size:70 - tcp_server.cpp:57
20250331 14:44:10.841654 388175 ERROR parse sericve_name[user] and method_name[Login] from full name [fixbug.user.Login] - rpc_dispatcher.cpp:235
20250331 14:44:10.841715 388175 INFO  79217084| get rpc requestname: "zhang san" pwd: "123456" - rpc_dispatcher.cpp:139
20250331 14:44:10.841817 388175 INFO  user name is zhang san - service.cpp:12
20250331 14:44:10.841855 388175 INFO  user password is 123456 - service.cpp:13
20250331 14:44:10.841891 388175 INFO  79217084| dispatch success, requesut->name: "zhang san" pwd: "123456", response->result { errMsg: "\344\270\200\345\210\207\346\255\243\345\270\270 very good" } sucess: true - rpc_dispatcher.cpp:163
20250331 14:44:10.843297 388175 INFO  127.0.0.1:58676客户断开了连接 - tcp_server.cpp:89
20250331 14:44:10.843378 388170 INFO  TcpServer::removeConnectionInLoop [networkServer] - connection networkServer-0.0.0.0:9001#1 - TcpServer.cc:109






#client
root@VM-16-3-ubuntu:~/projects/mprpc/bin# ./client ../conf/initConfigFile.conf
20240502 06:44:10.830904Z 388210 INFO  RpcChannel - rpc_channel.cpp:36
20240502 06:44:10.840230Z 388210 INFO  TcpClient::TcpClient[tcp_client] - connector 0x5606CDC8D750 - TcpClient.cc:69
20240502 06:44:10.840472Z 388210 INFO  TcpClient::connect[tcp_client] - connecting to 127.0.0.1:9001 - TcpClient.cc:107
20240502 06:44:10.840969Z 388210 INFO  79217084| call method name->fixbug.user.Login - rpc_channel.cpp:97
20240502 06:44:10.841118Z 388210 INFO  79217084 | connect success, peer addr[127.0.0.1:9001] - rpc_channel.cpp:127
20240502 06:44:10.841167Z 388210 INFO  79217084 | send rpc request success. call method name[fixbug.user.Login], peer addr[127.0.0.1:9001] - rpc_channel.cpp:132
20240502 06:44:10.842024Z 388211 INFO  接收到一个的包 data size:79 - tcp_client.cpp:33
20240502 06:44:10.842091Z 388210 INFO  79217084 | success get rpc response, call method name[fixbug.user.Login], peer addr[127.0.0.1:9001] - rpc_channel.cpp:166
20240502 06:44:10.842132Z 388210 INFO  79217084 | call rpc success, call method name[fixbug.user.Login], peer addr[127.0.0.1:9001] - rpc_channel.cpp:194
rpc call success:10一切正常 very good
```

## 项目解析

### TinyPB协议

TinyPB 是 TinyRPC 框架自定义的一种轻量化协议类型，它是基于 google 的 protobuf 而定制的，读者可以按需自行对协议格式进行扩充。

![image-20240501174650346](https://s2.loli.net/2024/05/01/e8n1PcKgwGXWbOy.png)

#### TinyPB 协议报文格式分解

> **TinyPb** 协议里面所有的 int 类型的字段在编码时都会先转为**网络字节序**！

```cpp
class TinyPBProtocol
{
public:
    /// @brief 包头标识,固定为0xFE
    static char     PB_START;
    /// @brief 包尾标识,固定为0xFF
    static char     PB_END;

public:
    /// @brief 整包长度
    int32_t         packageLen_{0};
    
    /// @brief rpc消息唯一标识id长度
    int32_t         msgIdLen_{0};
    /// @brief rpc消息唯一标识
    std::string     msgId_;

    /// @brief rpc方法名长度
    int32_t         methodNameLen_{0};
    /// @brief rpc方法名
    std::string     methodName_;

    /// @brief 错误码
    int32_t         errorCode_{0};
    /// @brief 错误信息长度
    int32_t         errorInfoLen_{0};
    /// @brief 错误信息
    std::string     errorInfo_;

    /// @brief 实际rpc请求数据
    std::string     pbData_;
    /// @brief 校验码
    int32_t         checksum_{0};

    /// @brief 判断是否已经被解析成功
    bool            parseSuccess_{false};
};
```

### 时序图

![solo-fetchupload-6893165705734529521-7932f433](https://s2.loli.net/2024/06/06/5YZd8x4gyL3K7tC.jpg)

### 错误码释义文档

err_code 详细说明如下表：

| **错误码**               | **错误代码** | **错误码描述**                                            |
| ------------------------ | ------------ | --------------------------------------------------------- |
| ERROR_PEER_CLOSED        | 10000000     | 连接时对端关闭                                            |
| ERROR_FAILED_CONNECT     | 10000001     | 连接失败                                                  |
| ERROR_FAILED_GET_REPLY   | 10000002     | RPC 调用未收到对端回包数据                                |
| ERROR_FAILED_DESERIALIZE | 10000003     | 反序列化失败，这种情况一般是 TinyPb 里面的 pb_data 有问题 |
| ERROR_FAILED_SERIALIZE   | 10000004     | 序列化失败                                                |
| ERROR_FAILED_ENCODE      | 10000005     | 编码失败                                                  |
| ERROR_FAILED_DECODE      | 10000006     | 解码失败                                                  |
| ERROR_RPC_CALL_TIMEOUT   | 10000007     | rpc 调用超时                                              |
| ERROR_SERVICE_NOT_FOUND  | 10000008     | service 不存在                                            |
| ERROR_METHOD_NOT_FOUND   | 10000009     | method 不存在 method                                      |
| ERROR_PARSE_SERVICE_NAME | 10000010     | service name 解析失败                                     |
| ERROR_RPC_CHANNEL_INIT   | 10000011     | rpc channel 初始化失败                                    |
| ERROR_RPC_PEER_ADDR      | 10000012     | rpc 调用时候对端地址异常                                  |
