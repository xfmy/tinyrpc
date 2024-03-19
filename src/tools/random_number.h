#include <random>

constexpr int MIN_RANDOM_NUMBER = 1000;
constexpr int MAX_RANDOM_NUMBER = 99999999;

std::string GetRandomNumber()
{
    std::random_device rd;
    std::default_random_engine eng(rd());
    std::uniform_int_distribution<uint32_t> distr(MIN_RANDOM_NUMBER,
                                                  MAX_RANDOM_NUMBER);
    return std::to_string(distr(eng));
}