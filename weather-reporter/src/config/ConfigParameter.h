#pragma once

#include <Arduino.h>
#include <IPAddress.h>

#include "Console.h"
#include "ByteBuffer.h"

namespace app {

class ConfigParameterBase {
public:
    static const unsigned char INVALID_ID = (-1);

    enum class Type {
        INVALID = 0,
        BYTE,
        NUMBER,
        STRING,
        IP_ADDRESS };

    ConfigParameterBase(unsigned char id, Type type);
    virtual ~ConfigParameterBase() {}

    unsigned char getId();
    Type getType();

    virtual ByteBuffer::iterator read(ByteBuffer::iterator& it);
    virtual ByteBuffer::iterator write(ByteBuffer::iterator& it);

    virtual bool isValid() = 0;

protected:
    unsigned char mId;
    Type mType;
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

    bool isValid() override;

private:
    T mValue;
    bool mIsValid;
};

template <typename T>
bool ConfigParameter<T>::isValid() {

    return mIsValid;
}

template <typename T>
ConfigParameter<T>::operator T() {

    return mValue;
}

template <typename T>
T &ConfigParameter<T>::operator=(const T& value) {

    this->mValue = value;
    return this->mValue;
}

} // namespace