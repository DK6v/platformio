#pragma once

#include <Byte.h>

namespace app
{

template <typename T>
class RBufferHelper
{
public:
    RBufferHelper(char(*cb)(T arg), T arg);

    bool getBit();
    uint32_t getBits(int count);

    uint8_t getByte();
    uint32_t getBytes(int count, bool reverse = false);

private:
    char (*cb_)(T);
    T arg_;

    uint16_t buffer_;
    uint16_t size_;
};

template <typename T>
RBufferHelper<T>::RBufferHelper(char(*cb)(T arg), T arg)
:   cb_(cb),
    arg_(arg),
    buffer_(0),
    size_(0) {}

template <typename T>
bool RBufferHelper<T>::getBit() {

    bool retval = false;
    uint8_t mask = 1;

    if (size_ == 0) {
        size_ = 8;
        buffer_ = cb_(arg_);
    }

    mask = 1 << (--size_);
    retval = (buffer_ & mask) ? true : false;
    buffer_ &= ~mask;

    return retval;
}

template <typename T>
uint32_t RBufferHelper<T>::getBits(int count) {

    uint32_t retval = 0;
    count = RANGE(count, 1, (BYTE_LEN * sizeof(uint32_t)));

    for (int ix = 0; ix < count; ++ix) {
        retval = (retval << 1) | getBit();
    }

    return retval;
}

template <typename T>
uint8_t RBufferHelper<T>::getByte() {

    return getBits(BYTE_LEN);
}

template <typename T>
uint32_t RBufferHelper<T>::getBytes(int count, bool reverse) {

    uint32_t retval = 0;
    count = RANGE(count, 1, sizeof(uint32_t));

    for (int ix = 0; ix < count; ++ix) {
        if (BYTE_XOR(IS_BIG_ENDIAN, reverse)) {
            // big endian
            reinterpret_cast<char*>(&retval)[(count - 1) - ix] = getByte();
        } else {
            // little endian
            reinterpret_cast<char*>(&retval)[ix] = getByte();
        }
    }

    return retval;
}

} // namespace