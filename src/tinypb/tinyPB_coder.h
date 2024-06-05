#pragma once
#include <string>
#include <memory>

namespace tinyrpc {
class TinyPBProtocol;
class TinyPBCoder
{
public:
    TinyPBCoder() = default;
    ~TinyPBCoder() = default;

    /**
     * @brief 对源数据进行编码,将数据转化为二进制字节码以供网络传输
     * 
     * @param message 源数据
     * @param target 目标,将message元数据编码后存储于其中
     */
    static void encode(std::shared_ptr<TinyPBProtocol> message, std::string& target);

    /**
     * @brief 解码,在字节流中解析出完整的报文,再将报文转换成TinyPB对象
     * 
     * @param message TinyPB对象,存放被解析后的数据
     * @param view 字节数据流
     * @return int 若解析出完整报文则返回包尾下标,否则返回-1
     */
    static int decode(std::shared_ptr<TinyPBProtocol> message,
                     std::string_view view);
};
} // namespace tinyrpc
