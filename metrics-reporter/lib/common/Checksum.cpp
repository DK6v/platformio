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
void ChecksumAlgorithm::accumulate(unsigned int* acc_p,
                                   const char value,
                                   unsigned short count) const {
        accumulate(acc_p, value);
};

void ChecksumAlgorithm::setBase(unsigned int* acc_p) const {
    *acc_p = 0;
};

void ChecksumAlgorithm::setFinal(unsigned int* acc_p) const {
    // do nothing
};

/* Class: Checksum
 */

unsigned int Checksum::calculate(const char* data, unsigned short length) const {
    return calculate(data, data + length);
}

unsigned int Checksum::calculate(const char* begin, const char* end) const {

    unsigned int retval = 0;

    mFunction.setBase(&retval);
    while (begin != end) {
        mFunction.accumulate(&retval, *begin++);
    }
    mFunction.setFinal(&retval);

    return retval;
}

/* Class: ChecksumXor
 */

void ChecksumXor::accumulate(unsigned int* acc_p, const char value) const {

    *acc_p ^= value;
}

/* Class: ChecksumCRC8
 */

ChecksumCRC8::ChecksumCRC8(const unsigned char polynom,
                           const unsigned char base,
                           const unsigned char finalXor,
                           const bool inReverse,
                           const bool outReverse)
    : ChecksumCRC(polynom, base, finalXor, inReverse, outReverse) {};

void ChecksumCRC8::accumulate(unsigned int* acc_p, const char value) const {

    *acc_p ^= (mInReverse) ? BYTE_REVERSE(value) : value;

    for (unsigned char ix = 0; ix < 8; ix++) {
        *acc_p = ((*acc_p & 0x80) ? ((*acc_p << 1) ^ mPolynom) : (*acc_p << 1)) & 0xFF;
    }
}

void ChecksumCRC8::setBase(unsigned int* acc_p) const {
    *acc_p = mBase;
}

void ChecksumCRC8::setFinal(unsigned int* acc_p) const {

    if (mOutReverse) {
        *acc_p = BYTE_REVERSE(NBYTE(0, *acc_p));
    }
    *acc_p ^= mFinalXor;
    *acc_p &= 0xFF;
}

/* Class: ChecksumCRC16
 */

ChecksumCRC16::ChecksumCRC16(const unsigned short polynom,
                             const unsigned short base,
                             const unsigned short finalXor,
                             const bool inReverse,
                             const bool outReverse)
    : ChecksumCRC(polynom,
                  base,
                  finalXor,
                  inReverse,
                  outReverse) {};

void ChecksumCRC16::accumulate(unsigned int* acc_p, const char value) const {

    *acc_p ^= ((mInReverse) ? BYTE_REVERSE(value) : value) << 8;

    for (unsigned char ix = 0; ix < 8; ++ix) {
        *acc_p = ((*acc_p & 0x8000) ? ((*acc_p << 1) ^ mPolynom) : (*acc_p << 1)) & 0xFFFF;
    }
}

void ChecksumCRC16::setBase(unsigned int* acc_p) const {
    *acc_p = mBase;
}

void ChecksumCRC16::setFinal(unsigned int* acc_p) const {

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

ChecksumCRC32& ChecksumCRC32::setPolynom(const unsigned int polynom) {

    mPolynom = (BYTE_REVERSE(NBYTE(3, mPolynom))) |
               (BYTE_REVERSE(NBYTE(2, mPolynom)) << 8) |
               (BYTE_REVERSE(NBYTE(1, mPolynom)) << 16) |
               (BYTE_REVERSE(NBYTE(0, mPolynom)) << 24);
    return *this;
};

void ChecksumCRC32::setBase(unsigned int* acc_p) const {
    *acc_p = mBase;
}

void ChecksumCRC32::setFinal(unsigned int* acc_p) const {

    if (!mOutReverse) {
        *acc_p = (BYTE_REVERSE(NBYTE(3, *acc_p))) |
                 (BYTE_REVERSE(NBYTE(2, *acc_p)) << 8) |
                 (BYTE_REVERSE(NBYTE(1, *acc_p)) << 16) |
                 (BYTE_REVERSE(NBYTE(0, *acc_p)) << 24);
    }

    *acc_p ^= mFinalXor;
}

void ChecksumCRC32::accumulate(unsigned int* acc_p, const char value) const {

    *acc_p ^= (!mInReverse) ? BYTE_REVERSE(value) : value;

    for (unsigned char ix = 0; ix < 8; ++ix) {
        *acc_p = (*acc_p >> 1) ^ (mPolynom & -((*acc_p) & 0x01));
    }
}

} // namespace