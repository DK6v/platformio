#include <EEPROM.h>

#include "ConfigParameter.h"
#include "Console.h"
#include "common/Checksum.h"

using namespace app;

// Class ConfigParameterBase

ConfigParameterBase::ConfigParameterBase(unsigned char id, Type type)
    : mId(id), mType(type) {}

unsigned char ConfigParameterBase::getId() {

    return this->mId;
}

ConfigParameterBase::Type ConfigParameterBase::getType() {

    return this->mType;
}

ByteBuffer::iterator ConfigParameterBase::read(ByteBuffer::iterator &it) {

    ByteBuffer::iterator nextIt = it;

    char id = *nextIt++;
    char type = *nextIt++;

    if (id != mId) {
        console.log("Skip parameter: id=%d (!= %d)", mId, id);
        return it;
    }

    if (type != static_cast<char>(mType)) {
        console.log("Skip parameter: id=%d, type=%d (!= %d)", mId, mType, type);
        return it;
    }

    return nextIt;
}

ByteBuffer::iterator ConfigParameterBase::write(ByteBuffer::iterator &it) {

    ByteBuffer::iterator nextIt = it;

    /* console.log("write: id=0x%x, type=0x%x", mId, mType); */

    *nextIt++ = static_cast<char>(mId);
    *nextIt++ = static_cast<char>(mType);

    return nextIt;
}

// Class ConfigParameter<char>

template <>
ConfigParameter<char>::ConfigParameter(unsigned char id)
    : ConfigParameterBase(id, Type::BYTE), mValue(0), mIsValid(false) {};

template <>
ConfigParameter<char>::ConfigParameter(unsigned char id, const char &value)
    : ConfigParameterBase(id, Type::BYTE), mValue(value), mIsValid(true) {};

template <>
ByteBuffer::iterator ConfigParameter<char>::read(ByteBuffer::iterator &it) {

    auto nextIt = ConfigParameterBase::read(it);

    if (nextIt != it) {

        mValue = *nextIt++;

        char checksum = Checksum(Checksum::CRC8)
            .calculate(reinterpret_cast<char *>(&mValue), sizeof(char));

        if (checksum != *nextIt++) {

            console.log("Invalid checksum (0x%X): id=%d, type=%d",
                        checksum, mId, mType);
            return it;
        }
    }

    return nextIt;
}

template <>
ByteBuffer::iterator ConfigParameter<char>::write(ByteBuffer::iterator &it) {

    auto nextIt = ConfigParameterBase::write(it);

    *nextIt++ = mValue;
    *nextIt++ = Checksum(Checksum::CRC8)
                    .calculate(reinterpret_cast<char *>(&mValue), sizeof(char));

    return nextIt;
}

// Class ConfigParameter<int>

template <>
ConfigParameter<int>::ConfigParameter(unsigned char id)
    : ConfigParameterBase(id, Type::NUMBER), mValue(0), mIsValid(false) {};

template <>
ConfigParameter<int>::ConfigParameter(unsigned char id, const int &value)
    : ConfigParameterBase(id, Type::NUMBER), mValue(value), mIsValid(true) {};

template <>
ByteBuffer::iterator ConfigParameter<int>::read(ByteBuffer::iterator &it) {

    auto nextIt = ConfigParameterBase::read(it);

    if (nextIt != it) {

        mValue = 0;

        for (unsigned char ix = 0; ix < sizeof(int); ++ix) {

            mValue |= BYTE_SET(ix, 0x00, *nextIt++);
        }

        char checksum = Checksum(Checksum::CRC8)
            .calculate(reinterpret_cast<char *>(&mValue),sizeof(int));

        if (checksum != *nextIt++) {

            console.log("Invalid checksum (0x%X): id=%d, type=%d",
                        checksum, mId, mType);
            return it;
        }

        console.log("CFG <= [%02d]: %d, CS: 0x%X", mId, mValue, checksum);
    }

    return nextIt;
}

template <>
ByteBuffer::iterator ConfigParameter<int>::write(ByteBuffer::iterator &it) {

    auto nextIt = ConfigParameterBase::write(it);

    char checksum = Checksum(Checksum::CRC8)
        .calculate(reinterpret_cast<char*>(&mValue), sizeof(int));

    for (unsigned char ix = 0; ix < sizeof(int); ++ix) {

        *nextIt++ = NBYTE(ix, mValue);
    }

    *nextIt++ = checksum; 

    console.log("CFG => [%02d]: %d, CS: 0x%X", mId, mValue, checksum);

    return nextIt;
}

// Class ConfigParameter<int>

template <>
ConfigParameter<std::string>::ConfigParameter(unsigned char id)
    : ConfigParameterBase(id, Type::STRING), mValue(""), mIsValid(false) {};

template <>
ConfigParameter<std::string>::ConfigParameter(unsigned char id,
                                              const std::string &value)
    : ConfigParameterBase(id, Type::STRING), mValue(value), mIsValid(true) {};

template <>
ByteBuffer::iterator
ConfigParameter<std::string>::read(ByteBuffer::iterator &it) {

    auto nextIt = ConfigParameterBase::read(it);

    if (nextIt != it) {

        char length = *nextIt++;

        mValue.clear();

        for (unsigned char ix = 0; ix < length; ++ix) {

            mValue += *nextIt++;
        }

        char checksum = Checksum(Checksum::CRC8)
            .calculate(mValue.c_str(), mValue.length());

        if (checksum != *nextIt++) {

            console.log("Invalid checksum (0x%X): id=%d, type=%d",
                        checksum, mId, mType);
            return it;
        }

        console.log("CFG <= [%02d]: '%s', CS: 0x%X", mId, mValue.c_str(), checksum);
    }

    return nextIt;
}

template <>
ByteBuffer::iterator
ConfigParameter<std::string>::write(ByteBuffer::iterator &it) {

    auto nextIt = ConfigParameterBase::write(it);

    char checksum = Checksum(Checksum::CRC8)
        .calculate(mValue.c_str(), mValue.length());

    *nextIt++ = mValue.length();

    for (unsigned char ix = 0; ix < mValue.length(); ++ix) {

        *nextIt++ = mValue[ix];
    }

    *nextIt++ = checksum;

    console.log("CFG => [%02d]: '%s', CS: 0x%X", mId, mValue.c_str(), checksum);

    return nextIt;
}

// Class ConfigParameter<IPAddress>

template <>
ConfigParameter<IPAddress>::ConfigParameter(unsigned char id)
    : ConfigParameterBase(id, Type::IP_ADDRESS), mValue(), mIsValid(false) {};

template <>
ConfigParameter<IPAddress>::ConfigParameter(unsigned char id,
                                            const IPAddress &value)
    : ConfigParameterBase(id, Type::IP_ADDRESS), mValue(value), mIsValid(true) {};

template <>
ByteBuffer::iterator
ConfigParameter<IPAddress>::read(ByteBuffer::iterator &it) {

    auto nextIt = ConfigParameterBase::read(it);

    if (nextIt != it) {

        unsigned int address = 0;

        for (unsigned char ix = 0; ix < sizeof(int); ++ix) {

            address |= BYTE_SET(ix, 0x00, *nextIt++);
        }

        char checksum = Checksum(Checksum::CRC8)
            .calculate(reinterpret_cast<char*>(&address), sizeof(int));

        if (checksum != static_cast<char>(*nextIt++)) {

            console.log("Invalid checksum (0x%X): id=%d, type=%d",
                        checksum, mId, mType);
            return it;
        }

        mValue = address;

        console.log("CFG <= [%02d]: %s, CS: 0x%X",
                    mId, IPAddress(address).toString().c_str(), checksum);
    }

    return nextIt;
}

template <>
ByteBuffer::iterator
ConfigParameter<IPAddress>::write(ByteBuffer::iterator &it) {

    auto nextIt = ConfigParameterBase::write(it);
    unsigned int address = static_cast<unsigned int>(mValue);

    for (unsigned char ix = 0; ix < sizeof(int); ++ix) {

        *nextIt++ = NBYTE(ix, address);
    }

    char checksum = Checksum(Checksum::CRC8)
        .calculate(reinterpret_cast<char*>(&address), sizeof(int));

    *nextIt++ = checksum;

    console.log("CFG => [%02d]: %s, CS: 0x%X",
                mId, mValue.toString().c_str(), checksum);

    return nextIt;
}