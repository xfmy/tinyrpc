// c++17
#include <source_location>
#include <exception>
#include <string>
class except final : std::exception
{
private:
    std::string msg;

public:
    except(const std::string errorMsg, std::source_location sourceInfo = std::source_location::current())
    {
        msg = msg + sourceInfo.file_name() + ' ' + sourceInfo.function_name() + ' ' + std::to_string(sourceInfo.line()) + ' ' + errorMsg;
    }
    std::string what()
    {
        return msg;
    }
};