#include "package.h"
#include <strings.h>
#include <muduo/net/Endian.h>
using namespace muduo::net::sockets;
const unsigned short package::head;

void package::encapsulation(const std::string& sourceData, std::string& target)
{
    
    target.clear();
    target.append(std::to_string(package::head));
    target.append(std::to_string(hostToNetwork32(sourceData.size())));
    target += sourceData;
    size_t checksum = std::hash<std::string>{}(sourceData);
    target.append(std::to_string(hostToNetwork32(checksum)));
}

int package::parse(std::string_view view, std::string &target)
{
    // 寻找包头
    size_t index = view.find(reinterpret_cast<const char *>(&package::head), sizeof package::head);
    if (index == std::string_view::npos)
        return -1;
    int size = view.size();
    //计算空包的大小
    int emptyPackageSize = index + sizeof(package::head) + sizeof(package::len) + sizeof(package::checksum);
    // 判断最低长度是否满足
    if (emptyPackageSize < size)
        return -1;
    index += sizeof package::head;
    //获取用户数据的长度
    package::len dataLen = networkToHost32(*(int *)(view.begin() + index));
    //判断缓冲区大小是否满足最低长度
    if (index - package::head + dataLen + emptyPackageSize > size)
        return -1;
    index += sizeof(package::len);
    target.assign(view.begin() + index, dataLen);
    index += dataLen;
    package::checksum checksum = networkToHost32(*(int *)(view.begin() + index));
    // 校验和
    if (checksum != std::hash<std::string>{}(target)){
        target.clear();
        return -1;
    }
    index += sizeof(package::checksum);
    return index;
}
