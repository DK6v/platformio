#include <cstdio>
#include <cstdarg>

#include "Console.h"

namespace app {

Console& console = Console::getInstance()
    .setSerial(Serial)
    .setBaudRate(115000);

Console::Console() : serial(Serial),
                     baudrate(115000),
                     mBuffer({0})
{
    this->serial.begin(this->baudrate);
};

Console::~Console() {}

Console& Console::setSerial(HardwareSerial &serial)
{
    this->serial = serial;
    this->serial.begin(this->baudrate);
    return *this;
}

Console& Console::setBaudRate(unsigned long baudrate)
{
    this->baudrate = baudrate;
    this->serial.begin(this->baudrate);
    return *this;
}

void Console::format(const char* fmt, ...) {

    char* npos = mBuffer + std::strlen(mBuffer);

    ::va_list ptr;
    ::va_start(ptr, fmt);
    std::vsnprintf(npos, sizeof(mBuffer) - std::strlen(mBuffer) - 1, fmt, ptr);
    ::va_end(ptr);
}

void Console::flush() {

    this->serial.println(mBuffer);
    mBuffer[0] = '\0';
}

void Console::log(const char* fmt, ...) {

    ::va_list ptr;
    ::va_start(ptr, fmt);
    std::vsnprintf(mBuffer, sizeof(mBuffer) - 1, fmt, ptr);
    ::va_end(ptr);

    this->flush();
}

}