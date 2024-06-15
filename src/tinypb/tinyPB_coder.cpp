//#include "muduo/base/Logging.h"
#include <muduo/net/Endian.h>
#include "tinyPB_coder.h"
#include "tinyPB_protocol.h"
#include "cstring"

using namespace muduo::net::sockets;
#define htonl(xx) hostToNetwork32(xx);
#define ntohl(xx) networkToHost32(xx);

namespace tinyrpc {
void TinyPBCoder::encode(std::shared_ptr<TinyPBProtocol> message,
                         std::string& target)
{
    //计算整个包长
    uint32_t packageLen =
        2 + 24 + message->msgId_.length() + message->methodName_.length() +
        message->errorInfo_.length() + message->pbData_.length();
    target.reserve(packageLen);
    target += TinyPBProtocol::PB_START;

    uint32_t packageLenNet = htonl(packageLen);
    target.append(reinterpret_cast<char*>(&packageLenNet),
                  sizeof(packageLenNet));

    uint32_t msgIdNet = htonl(message->msgId_.size());
    target.append(reinterpret_cast<char*>(&msgIdNet), sizeof(msgIdNet));
    target.append(message->msgId_);

    uint32_t methodNameNet = htonl(message->methodName_.size());
    target.append(reinterpret_cast<char*>(&methodNameNet),
                  sizeof(methodNameNet));
    target.append(message->methodName_);

    uint32_t errorCodeNet = htonl(message->errorCode_);
    target.append(reinterpret_cast<char*>(&errorCodeNet), sizeof(errorCodeNet));
    uint32_t errorInfoLenNet = htonl(message->errorInfoLen_);
    target.append(reinterpret_cast<char*>(&errorInfoLenNet),
                  sizeof(errorInfoLenNet));
    target.append(message->errorInfo_);

    target.append(message->pbData_);

    uint32_t checksumNet = htonl(std::hash<std::string>{}(message->pbData_));
    target.append(reinterpret_cast<char*>(&checksumNet), sizeof(checksumNet));
    target += TinyPBProtocol::PB_END;
}
int TinyPBCoder::decode(std::shared_ptr<TinyPBProtocol> message,
                        std::string_view view)
{
    int index = 0;
    int viewSize = view.size();
    while (index < viewSize && view[index++] != TinyPBProtocol::PB_START)
        ;
    if (index + 26 >= viewSize) return -1;
    uint32_t tmp = index;
    uint32_t packageLen = ntohl(
        *reinterpret_cast<uint32_t*>(const_cast<char*>(view.begin()) + index));
    if (packageLen > viewSize) return -1;
    index += sizeof(message->packageLen_);

    uint32_t msgIdLen = ntohl(
        *reinterpret_cast<uint32_t*>(const_cast<char*>(view.begin()) + index));
    index += sizeof(message->msgIdLen_);

    message->msgId_.assign(view.begin() + index, msgIdLen);
    index += msgIdLen;

    uint32_t methodNameLen = ntohl(
        *reinterpret_cast<uint32_t*>(const_cast<char*>(view.begin()) + index));
    index += sizeof(message->methodNameLen_);
    message->methodName_.assign(view.begin() + index, methodNameLen);
    index += methodNameLen;

    uint32_t errorCode = ntohl(
        *reinterpret_cast<uint32_t*>(const_cast<char*>(view.begin()) + index));
    index += sizeof(message->errorCode_);

    uint32_t errorInfoLen = ntohl(
        *reinterpret_cast<uint32_t*>(const_cast<char*>(view.begin()) + index));
    index += sizeof(message->errorInfoLen_);
    message->errorInfo_.assign(view.begin() + index, errorInfoLen);
    index += errorInfoLen;

    uint32_t protocDataLen =
        packageLen - 26 - msgIdLen - methodNameLen - errorInfoLen;
    message->pbData_.assign(view.begin() + index, protocDataLen);
    index += protocDataLen;

    uint32_t checksum;
    memcpy(&checksum, view.begin() + index, 4);
    checksum = ntohl(checksum);

    index += sizeof(message->checksum_);

    if (checksum !=
            static_cast<uint32_t>(std::hash<std::string>{}(message->pbData_)) ||
        view[index] != TinyPBProtocol::PB_END)
    {
        return decode(message,
                      std::string_view(view.begin() + tmp, viewSize - tmp));
    }

    message->packageLen_ = packageLen;
    message->msgIdLen_ = msgIdLen;
    message->methodNameLen_ = methodNameLen;
    message->errorCode_ = errorCode;
    message->errorInfoLen_ = errorInfoLen;
    message->checksum_ = checksum;
    index++;
    return index;
}
} // namespace tinyrpc
