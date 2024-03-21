#pragma once
#include <random>

constexpr int MIN_RANDOM_NUMBER = 1000;
constexpr int MAX_RANDOM_NUMBER = 99999999;

namespace mprpc{
std::string GetRandomNumberString();
}