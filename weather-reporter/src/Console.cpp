#include <cstdio>
#include <cstdarg>

#include "Console.h"

namespace app {

Console& console = Console::getInstance()
    .setSerial(Serial)
    .setBaudRate(115000);

Console::Console() : serial(Serial),
                     baudrate(115000)
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

void Console::log(std::string message) {
    this->serial.println(message.c_str());
}

void Console::log(const char* fmt, ...) {

    char buffer[64] = {0};

    ::va_list ptr;
    ::va_start(ptr, fmt);
    std::vsnprintf(buffer, sizeof(buffer) - 1, fmt, ptr);
    ::va_end(ptr);

    this->serial.println(buffer);
}

}