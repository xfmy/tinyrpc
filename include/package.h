/**
 * @file package.h
 * @brief 包的解析与封装
 * @details 包的结构设计 \
 *          包头(2bytes):固定头部0xFEFF \
 *          传输数据长度(4bytes) \
 *          实际传输数据(string) \
 *          校验和(8bytes)
 * @author xf
 * @version 1.0
 * @copyright BSD
 * @date 2023-11-27
 */

#pragma once
#include <string>
#include <optional>

#pragma pack(push)
#pragma pack(1)
class package
{
public:
    /// @brief 封包
    /// @details 为了避免字符串拷贝产生性能损耗,将传出参数target作为引用传入
    /// @param sourceData  需要封包的源数据
    /// @param target 输出目标
    static void encapsulation(const std::string &sourceData, std::string &target);

    /// @brief 解析数据,
    /// @param view 源数据
    /// @param target 输出目标
    /// @return 失败返回-1,成功返回包尾下标
    static int parse(std::string_view view, std::string& target);
private :
    // 0XFEFF 包头部	2字节
    static constexpr unsigned char head[2]{(unsigned char)0xFE, (unsigned char)0xFF};

    // 数据长度			4字节
    using len = uint32_t;

    // 传输数据
    // std::string data;

    // 校验和			8字节
    using checksum = uint64_t;
};
#pragma pack(pop)
