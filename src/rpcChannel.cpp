#include "rpcChannel.h"
#include "mprpcHeader.pb.h"
#include "mprpcNetwork.h"
#include "package.h"
#include "muduo/net/TcpClient.h"

#include <sys/types.h> /* See NOTES */
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
// void connect_(const TcpConnectionPtr & ptr){

// }

void RpcChannel::CallMethod(
    const PROTO::MethodDescriptor *method,
    PROTO::RpcController *controller,
    const PROTO::Message *request,
    PROTO::Message *response,
    PROTO::Closure *done)
{
    std::string sendData;
    std::string requestData;
    request->SerializeToString(&requestData);
    const PROTO::ServiceDescriptor *service = method->service();

    mprpcHeader::rpcHeader header;
    header.set_servicename(service->name());
    header.set_methodname(method->name());
    header.set_argssize(requestData.size());

    std::string headerData;
    header.SerializeToString(&headerData);
    int headerSize = headerData.size();
    sendData.append((const char *)&headerSize, 4);
    sendData.append(headerData);
    sendData.append(requestData);

    std::string target;
    package::encapsulation(sendData, target);

    int clientFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientFd == -1)
    {
        perror("socket error");
        exit(0);
    }
    sockaddr_in addr;
    addr.sin_port = htons(9001);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_family = PF_INET;

    int res = connect(clientFd, (sockaddr*)&addr, sizeof(addr));
    if(res == -1){
        perror("connect error:");
        exit(0);
    }
    int sendLen = send(clientFd, target.data(), target.size(), 0);
    char str[4096]{0};
    std::cout << "send data size:" << sendLen << std::endl;
    ssize_t len = recv(clientFd, str, 4096, 0);
    std::cout << "recv data size:" << len << std::endl;
    response->ParseFromString(std::string(str, len));

    close(clientFd);
}
