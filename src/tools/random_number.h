/**
 * @file random_number.h
 * @author xf
 * @brief 随机数字符串生成器
 * 
 */
#pragma once
#include <random>

constexpr int MIN_RANDOM_NUMBER = 1000;
constexpr int MAX_RANDOM_NUMBER = 99999999;

namespace tinyrpc{
/**
 * @brief 获取随机数字符串对象
 * 
 * @return std::string 
 */
std::string GetRandomNumberString();
}