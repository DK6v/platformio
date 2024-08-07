#pragma once

#include <string>

#include <Arduino.h>

namespace app {

class Console {
private:
    Console();
    ~Console();

    Console(Console const&) = delete;
    Console& operator= (Console const&) = delete;

public:
    static Console& getInstance() {
        static Console console;
        return console;
    }

    Console& setSerial(HardwareSerial& serial);
    Console& setBaudRate(unsigned long baudrate);

    void format(const char* fmt, ...);
    void flush();

    void log(const char* fmt, ...);

private:
    HardwareSerial& serial;
    unsigned long baudrate;

    char mBuffer[80];
};

extern Console& console;

}
