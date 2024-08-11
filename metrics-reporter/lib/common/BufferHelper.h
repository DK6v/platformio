#pragma once

#include <memory>
#include <iterator>
#include <functional>

#include "Byte.h"

namespace app
{

class RBufferHelper
{
public:
    RBufferHelper(std::function<char(void)> cb);

    bool getBit();
    uint32_t getBits(int count);

    uint8_t getByte();
    uint32_t getBytes(int count, bool reverse = false);

private:
    std::function<char(void)> cb_;

    uint16_t buffer_;
    uint16_t size_;
};

} // namespace