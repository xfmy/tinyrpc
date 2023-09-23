#pragma once
#include <map>
#include <string>

using configInfo = std::map<std::string, std::string>;

class RpcInitConfig
{
public:
    static configInfo execute();
};