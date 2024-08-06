#pragma once

#include <vector>
#include <iterator>

#include "Byte.h"

namespace app {

class ChecksumAlgorithm {
public:
    virtual void accumulate(unsigned int* acc_p, const char value) const = 0;

    virtual void accumulate(unsigned int* acc_p, const char value, unsigned short count) const;
    virtual void setBase(unsigned int* acc_p) const;
    virtual void setFinal(unsigned int* acc_p) const;
};

class ChecksumXor : public ChecksumAlgorithm {
public:
    explicit ChecksumXor() = default;

    virtual void accumulate(unsigned int* acc_p, const char value) const;
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

class ChecksumCRC8 : public ChecksumCRC<ChecksumCRC8, unsigned char> {
public:
    explicit ChecksumCRC8(const unsigned char polynom,
                          const unsigned char base = 0,
                          const unsigned char finalXor = 0,
                          const bool inReverse = false,
                          const bool outReverse = false);

    void accumulate(unsigned int* acc_p, const char value) const;

    void setBase(unsigned int* acc_p) const;
    void setFinal(unsigned int* acc_p) const;
};

class ChecksumCRC16 : public ChecksumCRC<ChecksumCRC16, unsigned short> {
public:

    explicit ChecksumCRC16(const unsigned short polynom,
                           const unsigned short base = 0,
                           const unsigned short finalXor = 0,
                           const bool inReverse = false,
                           const bool outReverse = false);

    void accumulate(unsigned int* acc_p, const char value) const;

    void setBase(unsigned int* acc_p) const;
    void setFinal(unsigned int* acc_p) const;
};

class ChecksumCRC32 : public ChecksumCRC<ChecksumCRC32, unsigned int>  {
public:

    explicit ChecksumCRC32(const unsigned int polynom,
                           const unsigned int base = 0,
                           const unsigned int finalXor = 0,
                           const bool inReverse = false,
                           const bool outReverse = false);

    ChecksumCRC32& setPolynom(const unsigned int polynom) override;

    void accumulate(unsigned int* acc_p, const char value) const;

    void setBase(unsigned int* acc_p) const;
    void setFinal(unsigned int* acc_p) const;
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
    unsigned int calculate(Iterator begin, Iterator end) const;

    unsigned int calculate(const char* begin, const char* end) const;
    unsigned int calculate(const char* data, unsigned short length) const;

    const ChecksumAlgorithm& mFunction;
};

template<typename Iterator>
unsigned int Checksum::calculate(Iterator begin, Iterator end) const {

    using value_type = typename std::iterator_traits<Iterator>::value_type;

    unsigned int retval = 0;
    unsigned int count = 0;

    mFunction.setBase(&retval);

    while (begin != end) {

        for (unsigned char ix = 0; ix < sizeof(value_type); ++ix) {

            const char value = NBYTE(ix, *begin);
            mFunction.accumulate(&retval, value, count++);
        }

        begin++;
    }

    mFunction.setFinal(&retval);
    return retval;
}


} // namespace