#pragma once

#include <list>
#include <array>
#include <string>
#include <sstream>

#include <Arduino.h>
#include "OneWire.h"

namespace app {

class TimeRFC868 {   
public:
    TimeRFC868(const char* host, uint16_t port);
    TimeRFC868(const std::string& host, uint16_t port);
    ~TimeRFC868() = default;

    unsigned int getCurrentTime();

private:
    enum Status {
        CONNECTED = 0,
        FAILED,
        UNKNOWN
    };
private:
    std::string mHost;
    uint16_t mPort;

    Status mStatus;
};

} // namespace fm