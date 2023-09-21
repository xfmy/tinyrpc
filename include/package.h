#pragma once
#include <string>
#include <optional>
#define MAX_BUF 4096

// 包的设计类
/*************************************************************************************
 *9 + data（长度）
 * UINT parse(char* buf, UINT& strSz)
 *	参数：（缓冲区，数据大小）
 *	功能： 解析包的数据，
 *	返回值：0-》解析失败，其他-》buf使用大小
 ***************************************************************************************/
#pragma pack(push)
#pragma pack(1)
class package
{
public:
    //封包
    static void encapsulation(const std::string &sourceData, std::string &target);

    //解析数据,失败返回-1,成功返回包尾下标
    static int parse(std::string_view view, std::string& target);
private :
    // 0XFEFF 头部	2字节
    static const unsigned short head = 0xFEFF;

    // 长度			4字节
    //int len;
    using len = int;

    // 传输数据
    // std::string data;

    // 校验和			4字节
    // int checksum;
    using checksum = int;
};
#pragma pack(pop)
