#pragma once
#include <string>
#include <memory>

namespace mprpc {
class TinyPBProtocol;
class TinyPBCoder
{
public:
    TinyPBCoder() = default;
    ~TinyPBCoder() = default;

    static void encode(std::shared_ptr<TinyPBProtocol> message, std::string& target);
    static int decode(std::shared_ptr<TinyPBProtocol> message,
                     std::string_view view);
};
} // namespace mprpc
