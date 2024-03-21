#include <libconfig.h++>
#include <muduo/base/Logging.h>

#include "rpc_init_config.h"

using namespace std::string_literals;

namespace mprpc {
configInfo RpcInitConfig::execute()
{
    libconfig::Config config;
    configInfo info;
    /*
    解析配置文件。
    */
    try
    {
        config.readFile("../conf/initConfigFile.conf");
    }
    catch (const libconfig::FileIOException &fioex)
    {
        LOG_FATAL << "I/O error while reading file.";
        exit(EXIT_FAILURE);
    }
    catch (const libconfig::ParseException &pex)
    {
        LOG_FATAL << "Parse error at "s + pex.getFile() + ":" + std::to_string(pex.getLine()) + " - " + pex.getError();
        exit(EXIT_FAILURE);
    }

    /* 从配置文件中，得到日志相关配置值 */
    try
    {
        std::string servicePort = config.lookup("config.servicePort");
        info.emplace("servicePort", servicePort);

        std::string servicePublicIp = config.lookup("config.servicePublicIp");
        info.emplace("servicePublicIp", servicePublicIp);

        std::string consulIp = config.lookup("config.consulIp");
        info.emplace("consulIp", consulIp);

        std::string consulPort = config.lookup("config.consulPort");
        info.emplace("consulPort", consulPort);
    }
    catch (const libconfig::SettingNotFoundException &nfex)
    {
        LOG_FATAL << "log setting mistake in configuration file." << nfex.what();
    }
    return info;
}
}