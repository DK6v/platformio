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

class WBufferHelper
{
public:
    WBufferHelper(std::function<void(char)> cb);

    void setByte(uint8_t value);
    void setBytes(uint32_t value, int count, bool reverse = false);

    void setBit(bool value);
    void setBits(uint32_t value, int count);

    void flush();
    void spoof();

//    uint32_t setBits(int count);

//    uint8_t setByte();
//    uint32_t setBytes(int count, bool reverse = false);

private:
    std::function<void(char)> cb_;

    uint16_t buffer_;
    uint16_t size_;
};

} // namespace