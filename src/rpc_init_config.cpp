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
        const libconfig::Setting &config = cfg.getRoot()["config"];

        // 遍历所有配置项
        // for (int i = 0; i < config.getLength(); ++i)
        // {
        //     const libconfig::Setting &setting = config[i];
        //     const std::string name = setting.getName();
        //     std::string value;
        //     setting.lookupValue(name, value); // 获取配置项的值
        //     info.emplace(name, value);
        // }
        // 使用迭代器遍历所有字段
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
    /* 从配置文件中，得到日志相关配置值 */
    // try
    // {
    //     std::string servicePort = config.lookup("config.servicePort");
    //     info.emplace("servicePort", servicePort);

    //     std::string servicePublicIp =
    //     config.lookup("config.servicePublicIp");
    //     info.emplace("servicePublicIp", servicePublicIp);

    //     std::string consulIp = config.lookup("config.consulIp");
    //     info.emplace("consulIp", consulIp);

    //     std::string consulPort = config.lookup("config.consulPort");
    //     info.emplace("consulPort", consulPort);
    // }
    // catch (const libconfig::SettingNotFoundException &nfex)
    // {
    //     LOG_FATAL << "log setting mistake in configuration file." <<
    //     nfex.what();
    // }
    return info;
}
} // namespace tinyrpc