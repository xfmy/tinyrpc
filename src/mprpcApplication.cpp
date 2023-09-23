#include "mprpcApplication.h"

void MprpcApplication::init()
{
    infoMap = RpcInitConfig::execute();
}

std::optional<std::string> MprpcApplication::atConfigItem(std::string field)
{
    std::optional<std::string> ret;
    auto it = infoMap.find(field);
    if(it != infoMap.end()){
        ret = it->second;
    }
    return ret;
}
