#include "random_number.h"

namespace mprpc {
std::string GetRandomNumberString()
{
    std::random_device rd;
    std::default_random_engine eng(rd());
    std::uniform_int_distribution<uint32_t> distr(MIN_RANDOM_NUMBER,
                                                  MAX_RANDOM_NUMBER);
    return std::to_string(distr(eng));
}
}