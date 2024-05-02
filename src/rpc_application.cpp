#include "rpc_application.h"

namespace mprpc
{
void RpcApplication::init(std::string path)
{
    infoMap = RpcInitConfig::execute(path);
}

std::optional<std::string> RpcApplication::atConfigItem(std::string field) const {
    std::optional<std::string> ret;
    auto it = infoMap.find(field);
    if(it != infoMap.end()){
        ret = it->second;
    }
    return ret;
}
}