#pragma once

#include <Byte.h>

namespace app
{

template <typename T>
class WBufferHelper
{
public:
    WBufferHelper(void(*cb)(char, T arg), T arg);

    void setByte(uint8_t value);
    void setBytes(uint32_t value, int count, bool reverse = false);

    void setBit(bool value);
    void setBits(uint32_t value, int count);

    void flush();
    void spoof();

private:
    void (*cb_)(char, T);
    T arg_;

    uint16_t buffer_;
    uint16_t size_;
};

template <typename T>
WBufferHelper<T>::WBufferHelper(void(*cb)(char, T arg), T arg)
:   cb_(cb),
    arg_(arg),
    buffer_(0),
    size_(0) {}

template <typename T>
void WBufferHelper<T>::setByte(uint8_t value) {

    setBits(value, 8);
}

template <typename T>
void WBufferHelper<T>::setBytes(uint32_t value, int count, bool reverse) {

    count = RANGE(count, 1, sizeof(uint32_t));

    for (int ix = 0; ix < count; ++ix) {
        if (BYTE_XOR(IS_BIG_ENDIAN, reverse)) {
             // big endian
            setByte(reinterpret_cast<char*>(&value)[(count - 1) - ix]);
        } else {
             // little endian
            setByte(reinterpret_cast<char*>(&value)[ix]);
        }
    }
}

template <typename T>
void WBufferHelper<T>::setBit(bool value) {

    buffer_ = (buffer_ << 1) + static_cast<uint8_t>(value);

    if (++size_ == BYTE_LEN) {
        flush();
    }
}

template <typename T>
void WBufferHelper<T>::setBits(uint32_t value, int count) {

    count = RANGE(count, 1, (BYTE_LEN * sizeof(uint32_t)));

    for (int ix = 0; ix < count; ++ix) {
        setBit((value >> ((count - 1) - ix)) & 0x01);
    }
}

template <typename T>
void WBufferHelper<T>::flush() {

    cb_(buffer_, arg_);

    size_ = 0;
    buffer_ = 0;
}

template <typename T>
void WBufferHelper<T>::spoof() {

    buffer_ = buffer_ << (BYTE_LEN - size_);

    flush();
}

} // namespace