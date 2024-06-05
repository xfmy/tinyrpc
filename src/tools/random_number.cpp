#include "random_number.h"

namespace tinyrpc {
std::string GetRandomNumberString()
{
    //cpp11 随机数生成器
    std::random_device rd;
    std::default_random_engine eng(rd());   
    std::uniform_int_distribution<uint32_t> distr(MIN_RANDOM_NUMBER,
                                                  MAX_RANDOM_NUMBER);
    return std::to_string(distr(eng));
}
}