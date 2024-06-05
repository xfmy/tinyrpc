#include <cstring>
#include <memory>
#include <iostream>
#include "../src/tinypb/tinyPB_coder.h"
#include "../src/tinypb/tinyPB_protocol.h"

int main()
{
    std::shared_ptr<tinyrpc::TinyPBProtocol> tinyPtr(std::make_shared<tinyrpc::TinyPBProtocol>());
    tinyPtr->msgId_ = "666";
    tinyPtr->methodName_ = "func";
    tinyPtr->pbData_ = "hello world";

    std::string msg;
    tinyrpc::TinyPBCoder::encode(tinyPtr,msg);
    std::shared_ptr<tinyrpc::TinyPBProtocol> targetPtr(std::make_shared<tinyrpc::TinyPBProtocol>());
    tinyrpc::TinyPBCoder::decode(targetPtr,msg);
    if (tinyPtr->msgId_ == targetPtr->msgId_ &&
        tinyPtr->methodName_ == targetPtr->methodName_ &&
        tinyPtr->pbData_ == targetPtr->pbData_)
    {
        std::cout << "The custom tinypb protocol serialization and "
                     "deserialization tests pass\n";
    }
    else
        std::cout << "The custom tinypb protocol serialization and "
                     "deserialization tests failed\n";
    return 0;
}
