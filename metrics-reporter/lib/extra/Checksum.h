#pragma once

#include <vector>
#include <iterator>

#include "Byte.h"

namespace app {

class ChecksumAlgorithm {
public:
    virtual void accumulate(uint32_t* acc_p, const char value) const = 0;

    virtual void accumulate(uint32_t* acc_p, const char value, uint16_t count) const;
    virtual void setBase(uint32_t* acc_p) const;
    virtual void setFinal(uint32_t* acc_p) const;
};

class ChecksumXor : public ChecksumAlgorithm {
public:
    explicit ChecksumXor() = default;

    virtual void accumulate(uint32_t* acc_p, const char value) const;
};

template <typename D, typename T>
class ChecksumCRC : public ChecksumAlgorithm {
public:
    ChecksumCRC(const T polynom,
                const T base = 0,
                const T finalXor = 0,
                const bool inReverse = false,
                const bool outReverse = false)
        : mPolynom(polynom),
          mBase(base),
          mFinalXor(finalXor),
          mInReverse(inReverse),
          mOutReverse(outReverse) {}

    virtual D& setPolynom(const T polynom) {
        mPolynom = polynom;
        return *(static_cast<D*>(this));
    };

    D& setBase(const T base) {
        mBase = base;
        return *(static_cast<D*>(this));
    };

    D& setFinalXor(const T finalXor) {
        mFinalXor = finalXor;
        return *(static_cast<D*>(this));
    };

    D& setInReverse(const bool inReverse = true) {
        mInReverse = inReverse;
        return *(static_cast<D*>(this));
    };

    D& setOutReverse(const bool outReverse = true) {
        mOutReverse = outReverse;
        return *(static_cast<D*>(this));
    };

protected:
    T mPolynom;
    T mBase;
    T mFinalXor;
    bool mInReverse;
    bool mOutReverse;
};

class ChecksumCRC8 : public ChecksumCRC<ChecksumCRC8, uint8_t> {
public:
    explicit ChecksumCRC8(const uint8_t polynom,
                          const uint8_t base = 0,
                          const uint8_t finalXor = 0,
                          const bool inReverse = false,
                          const bool outReverse = false);

    void accumulate(uint32_t* acc_p, const char value) const;

    void setBase(uint32_t* acc_p) const;
    void setFinal(uint32_t* acc_p) const;
};

class ChecksumCRC16 : public ChecksumCRC<ChecksumCRC16, uint16_t> {
public:

    explicit ChecksumCRC16(const uint16_t polynom,
                           const uint16_t base = 0,
                           const uint16_t finalXor = 0,
                           const bool inReverse = false,
                           const bool outReverse = false);

    void accumulate(uint32_t* acc_p, const char value) const;

    void setBase(uint32_t* acc_p) const;
    void setFinal(uint32_t* acc_p) const;
};

class ChecksumCRC32 : public ChecksumCRC<ChecksumCRC32, uint32_t>  {
public:

    explicit ChecksumCRC32(const uint32_t polynom,
                           const uint32_t base = 0,
                           const uint32_t finalXor = 0,
                           const bool inReverse = false,
                           const bool outReverse = false);

    ChecksumCRC32& setPolynom(const uint32_t polynom) override;

    void accumulate(uint32_t* acc_p, const char value) const;

    void setBase(uint32_t* acc_p) const;
    void setFinal(uint32_t* acc_p) const;
};

class Checksum {
public:
    static ChecksumXor XOR;
    static ChecksumCRC8 CRC8;
    static ChecksumCRC16 CRC16;
    static ChecksumCRC32 CRC32;

    Checksum(const ChecksumAlgorithm& function): mFunction(function) {}
    ~Checksum() = default;

    template<typename Iterator>
    uint32_t calculate(Iterator begin, Iterator end) const;

    uint32_t calculate(const char* begin, const char* end) const;
    uint32_t calculate(const char* data, uint16_t length) const;

    const ChecksumAlgorithm& mFunction;
};

template<typename Iterator>
uint32_t Checksum::calculate(Iterator begin, Iterator end) const {

    using value_type = typename std::iterator_traits<Iterator>::value_type;

    uint32_t retval = 0;
    uint32_t count = 0;

    mFunction.setBase(&retval);

    while (begin != end) {

        for (uint8_t ix = 0; ix < sizeof(value_type); ++ix) {

            const char value = NBYTE(ix, *begin);
            mFunction.accumulate(&retval, value, count++);
        }

        begin++;
    }

    mFunction.setFinal(&retval);
    return retval;
}

} // namespace