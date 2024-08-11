#include <memory>
#include <cstring>
#include <algorithm>

#include "Byte.h"
#include "BufferHelper.h"

namespace app {

// Class RBufferHelper

RBufferHelper::RBufferHelper(std::function<char(void)> cb)
:   cb_(cb),
    buffer_(0),
    size_(0) {}

bool RBufferHelper::getBit() {
    bool retval = false;
    uint8_t mask = 1;

    if (size_ == 0) {
        size_ = 8;
        buffer_ = cb_();
    }

    mask = 1 << (--size_);
    retval = (buffer_ & mask) ? true : false;
    buffer_ &= ~mask;

    return retval;
}

uint32_t RBufferHelper::getBits(int count) {
    uint32_t retval = 0;
    count = RANGE_LIMIT(count, 1, (BYTE_LEN * sizeof(uint32_t)));

    for (int ix = 0; ix < count; ++ix) {
        retval = (retval << 1) | getBit();
    }

    return retval;
}

uint8_t RBufferHelper::getByte() {
    return getBits(BYTE_LEN);
}

uint32_t RBufferHelper::getBytes(int count, bool reverse) {
    uint32_t retval = 0;
    count = RANGE_LIMIT(count, 1, sizeof(uint32_t));

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