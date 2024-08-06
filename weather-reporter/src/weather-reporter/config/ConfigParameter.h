#pragma once

#include <type_traits>
#include <vector>

#include <Arduino.h>
#include <IPAddress.h>

#include "Console.h"
#include "ByteBuffer.h"
#include "Checksum.h"

namespace app {

enum class ConfigParameterType {
    INVALID = 0,
    BYTE,
    NUMBER,
    STRING,
    IP_ADDRESS,
    ARRAY
};

class ConfigParameterBase {
public:
    static const unsigned char INVALID_ID = (-1);

    ConfigParameterBase(ConfigParameterType type, unsigned char id, bool isValid);
    virtual ~ConfigParameterBase() {}

    unsigned char getId();
    ConfigParameterType getType();

    bool isValid();

    virtual ByteBuffer::iterator read(ByteBuffer::iterator& it);
    virtual ByteBuffer::iterator write(ByteBuffer::iterator& it);

protected:
    ConfigParameterType mType;
    unsigned char mId;
    bool mIsValid;
};

template <typename T>
class ConfigParameter : public ConfigParameterBase {
public:
    ConfigParameter(unsigned char id = INVALID_ID);
    ConfigParameter(unsigned char id, const T& value);

    operator T();
    T &operator=(const T &value);

    ByteBuffer::iterator read(ByteBuffer::iterator& it) override;
    ByteBuffer::iterator write(ByteBuffer::iterator& it) override;

private:
    T mValue;
    bool mIsValid;
};

template <typename T>
ConfigParameter<T>::operator T() {

    return mValue;
}

template <typename T>
T &ConfigParameter<T>::operator=(const T& value) {

    this->mValue = value;
    return this->mValue;
}

// Class ConfigParameter<vector<T>>

template <typename T>
class ConfigParameter<std::vector<T>> : public ConfigParameterBase {
public:
    ConfigParameter(unsigned char id = INVALID_ID);
    ConfigParameter(unsigned char id, const std::vector<T>& value);

    operator std::vector<T>();
    std::vector<T> &operator=(const std::vector<T> &value);

    ByteBuffer::iterator read(ByteBuffer::iterator& it) override;
    ByteBuffer::iterator write(ByteBuffer::iterator& it) override;

private:
    std::vector<T> mValue;
    bool mIsValid;
};

template <typename T>
ConfigParameter<std::vector<T>>::ConfigParameter(unsigned char id)
    : ConfigParameterBase(ConfigParameterType::ARRAY, id, false), mValue({}) {

    static_assert(std::is_arithmetic<T>::value, "Not an arithmetic type");
};

template <typename T>
ConfigParameter<std::vector<T>>::ConfigParameter(unsigned char id,
                                                 const std::vector<T> &value)
    : ConfigParameterBase(ConfigParameterType::ARRAY, id, true), mValue(value) {

    static_assert(std::is_arithmetic<T>::value, "Not an arithmetic type");
};

template <typename T>
ConfigParameter<std::vector<T>>::operator std::vector<T>() {

    return mValue;
}

template <typename T>
std::vector<T> &ConfigParameter<std::vector<T>>::operator=(const std::vector<T>& value) {

    this->mValue = value;
    return this->mValue;
}

template <typename T>
ByteBuffer::iterator
ConfigParameter<std::vector<T>>::read(ByteBuffer::iterator &it) {

    auto nextIt = ConfigParameterBase::read(it);

    if (nextIt != it) {

        char length = *nextIt++;

        mValue.clear();

        for (unsigned char ix = 0; ix < length; ++ix) {

            T currValue = 0;

            for (unsigned char ix = 0; ix < sizeof(T); ++ix) {
                currValue |= BYTE_SET(ix, 0x00, *nextIt++);
            }

            mValue.push_back(currValue);
        }

        char checksum = Checksum(Checksum::CRC8)
            .calculate<typename std::vector<T>::iterator>(mValue.begin(), mValue.end());

        if (checksum != *nextIt++) {

            console.log("Invalid checksum (0x%X): id=%d, type=%d",
                        checksum, mId, mType);
            return it;
        }

        console.format("CFG <= R [%02d]:", mId);
        for (auto &value: mValue) {
            console.format(" %x", value);
        }
        console.format(", sz=%d, CS=0x%X", mValue.size(), checksum);
        console.flush();
    }

    return nextIt;
}

template <typename T>
ByteBuffer::iterator
ConfigParameter<std::vector<T>>::write(ByteBuffer::iterator &it) {

    auto nextIt = ConfigParameterBase::write(it);

    char checksum = Checksum(Checksum::CRC8)
        .calculate<typename std::vector<T>::iterator>(mValue.begin(), mValue.end());

    *nextIt++ = mValue.size();

    for (unsigned char ix = 0; ix < mValue.size(); ++ix) {
        for (unsigned char jx = 0; jx < sizeof(T); ++jx) {

            *nextIt++ = mValue[ix];
        }
    }

    *nextIt++ = checksum;
    
    console.format("CFG => W [%02d]:", mId);
    for (auto &value: mValue) {
        console.format(" %x", value);
    }
    console.format(", sz=%d, CS=0x%X", mValue.size(), checksum);
    console.flush();

    return nextIt;
}

} // namespace