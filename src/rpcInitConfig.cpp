#include "rpcInitConfig.h"
#include <libconfig.h++>
#include <muduo/base/Logging.h>
using namespace std::string_literals;

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
        std::string serverPort = config.lookup("baseConf.serverPort");
        info.emplace("serverPort", serverPort);

        std::string zookeeperIp = config.lookup("baseConf.zookeeperIp");
        info.emplace("zookeeperIp", zookeeperIp);

        std::string zookeeperPort = config.lookup("baseConf.zookeeperPort");
        info.emplace("zookeeperPort", zookeeperPort);
    }
    catch (const libconfig::SettingNotFoundException &nfex)
    {
        LOG_FATAL << "log setting mistake in configuration file." << nfex.what();
    }
    return info;
}