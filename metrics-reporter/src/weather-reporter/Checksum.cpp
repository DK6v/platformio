#include <iterator>

#include "Byte.h"
#include "Checksum.h"

namespace app {

ChecksumXor Checksum::XOR = ChecksumXor();
ChecksumCRC8 Checksum::CRC8 = ChecksumCRC8(0x07);
ChecksumCRC16 Checksum::CRC16 = ChecksumCRC16(0x8005);
ChecksumCRC32 Checksum::CRC32 = ChecksumCRC32(0x04C11DB7, 0xFFFFFFFF, 0xFFFFFFFF, true, true);

/* Class: ChecksumAlgorithm
 */
void ChecksumAlgorithm::accumulate(uint32_t* acc_p,
                                   const char value,
                                   uint16_t count) const {
    (void)count;
    accumulate(acc_p, value);
};

void ChecksumAlgorithm::setBase(uint32_t* acc_p) const {
    *acc_p = 0;
};

void ChecksumAlgorithm::setFinal(uint32_t* acc_p) const {
    (void)acc_p;
    // do nothing
};

/* Class: Checksum
 */

uint32_t Checksum::calculate(const char* data, uint16_t length) const {
    return calculate(data, data + length);
}

uint32_t Checksum::calculate(const char* begin, const char* end) const {

    uint32_t retval = 0;

    mFunction.setBase(&retval);
    while (begin != end) {
        mFunction.accumulate(&retval, *begin++);
    }
    mFunction.setFinal(&retval);

    return retval;
}

/* Class: ChecksumXor
 */

void ChecksumXor::accumulate(uint32_t* acc_p, const char value) const {

    *acc_p ^= value;
}

/* Class: ChecksumCRC8
 */

ChecksumCRC8::ChecksumCRC8(const uint8_t polynom,
                           const uint8_t base,
                           const uint8_t finalXor,
                           const bool inReverse,
                           const bool outReverse)
: ChecksumCRC(polynom, base, finalXor, inReverse, outReverse) {};

void ChecksumCRC8::accumulate(uint32_t* acc_p, const char value) const {

    *acc_p ^= (mInReverse) ? BYTE_REVERSE(value) : value;

    for (uint8_t ix = 0; ix < 8; ix++) {
        *acc_p = ((*acc_p & 0x80) ? ((*acc_p << 1) ^ mPolynom) : (*acc_p << 1)) & 0xFF;
    }
}

void ChecksumCRC8::setBase(uint32_t* acc_p) const {
    *acc_p = mBase;
}

void ChecksumCRC8::setFinal(uint32_t* acc_p) const {

    if (mOutReverse) {
        *acc_p = BYTE_REVERSE(NBYTE(0, *acc_p));
    }
    *acc_p ^= mFinalXor;
    *acc_p &= 0xFF;
}

/* Class: ChecksumCRC16
 */

ChecksumCRC16::ChecksumCRC16(const uint16_t polynom,
                             const uint16_t base,
                             const uint16_t finalXor,
                             const bool inReverse,
                             const bool outReverse)
: ChecksumCRC(polynom, base, finalXor, inReverse, outReverse) {};

void ChecksumCRC16::accumulate(uint32_t* acc_p, const char value) const {

    *acc_p ^= ((mInReverse) ? BYTE_REVERSE(value) : value) << 8;

    for (uint8_t ix = 0; ix < 8; ++ix) {
        *acc_p = ((*acc_p & 0x8000) ? ((*acc_p << 1) ^ mPolynom) : (*acc_p << 1)) & 0xFFFF;
    }
}

void ChecksumCRC16::setBase(uint32_t* acc_p) const {
    *acc_p = mBase;
}

void ChecksumCRC16::setFinal(uint32_t* acc_p) const {

    if (mOutReverse) {
        *acc_p = (BYTE_REVERSE(NBYTE(1, *acc_p))) |
                 (BYTE_REVERSE(NBYTE(0, *acc_p)) << 8);
    }
    *acc_p ^= mFinalXor;
    *acc_p &= 0xFFFF;
}

/* Class: ChecksumCRC32
 */

ChecksumCRC32::ChecksumCRC32(const uint32_t polynom,
                             const uint32_t base,
                             const uint32_t finalXor,
                             const bool inReverse,
                             const bool outReverse)
: ChecksumCRC(polynom, base, finalXor, inReverse, outReverse) {

    setPolynom(polynom);
}

ChecksumCRC32& ChecksumCRC32::setPolynom(const uint32_t polynom) {

    mPolynom = (static_cast<uint32_t>(BYTE_REVERSE(NBYTE(3, polynom)))) |
               (static_cast<uint32_t>(BYTE_REVERSE(NBYTE(2, polynom))) << 8) |
               (static_cast<uint32_t>(BYTE_REVERSE(NBYTE(1, polynom))) << 16) |
               (static_cast<uint32_t>(BYTE_REVERSE(NBYTE(0, polynom))) << 24);
    return *this;
};

void ChecksumCRC32::setBase(uint32_t* acc_p) const {
    *acc_p = mBase;
}

void ChecksumCRC32::setFinal(uint32_t* acc_p) const {

    if (!mOutReverse) {
        *acc_p = (static_cast<uint32_t>(BYTE_REVERSE(NBYTE(3, *acc_p)))) |
                 (static_cast<uint32_t>(BYTE_REVERSE(NBYTE(2, *acc_p))) << 8) |
                 (static_cast<uint32_t>(BYTE_REVERSE(NBYTE(1, *acc_p))) << 16) |
                 (static_cast<uint32_t>(BYTE_REVERSE(NBYTE(0, *acc_p))) << 24);
    }

    *acc_p ^= mFinalXor;
}

void ChecksumCRC32::accumulate(uint32_t* acc_p, const char value) const {

    *acc_p ^= (!mInReverse) ? BYTE_REVERSE(value) : value;

    for (uint8_t ix = 0; ix < 8; ++ix) {
        *acc_p = (*acc_p >> 1) ^ (mPolynom & -((*acc_p) & 0x01));
    }
}

} // namespace