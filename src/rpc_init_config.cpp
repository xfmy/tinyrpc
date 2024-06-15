#include <libconfig.h++>
#include <muduo/base/Logging.h>

#include "rpc_init_config.h"

using namespace std::string_literals;

namespace tinyrpc {
configInfo RpcInitConfig::execute(std::string path)
{
    libconfig::Config cfg;
    configInfo info;
    /*
    解析配置文件。
    */
    try
    {
        // 加载配置文件
        cfg.readFile(path);
    }
    catch (const libconfig::FileIOException &fioex)
    {
        LOG_FATAL << "I/O error while reading file.";
        exit(EXIT_FAILURE);
    }
    catch (const libconfig::ParseException &pex)
    {
        LOG_FATAL << "Parse error at "s + pex.getFile() + ":" +
                         std::to_string(pex.getLine()) + " - " + pex.getError();
        exit(EXIT_FAILURE);
    }

    try
    {
        // 获取根目录下所有配置字段集合
        const libconfig::Setting &config = cfg.getRoot()["config"];
        // 使用迭代器遍历所有字段并加入集合
        for (auto it = config.begin(); it != config.end(); ++it)
        {
            const libconfig::Setting &field = *it;
            const std::string name = field.getName();
            const std::string value = field.c_str();
            info.emplace(name, value);
        }
    }
    catch (const libconfig::SettingException &err)
    {
        LOG_FATAL << err.what();
        exit(EXIT_FAILURE);
    }
    return info;
}
} // namespace tinyrpc