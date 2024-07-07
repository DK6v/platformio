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

    void log(std::string message);

    // void log(const char* message);
    void log(const char* fmt, ...);

private:
    HardwareSerial& serial;

    unsigned long baudrate;
};

extern Console& console;

}
