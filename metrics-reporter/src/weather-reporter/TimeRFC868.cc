#include <list>
#include <array>
#include <string>
#include <sstream>

#include <Arduino.h>
#include <WiFiClient.h>

#include "Timer.h"
#include "TimeRFC868.h"

namespace app {

TimeRFC868::TimeRFC868(const std::string& host, uint16_t port = 37) :
    mHost(host),
    mPort(port),
    mStatus(Status::UNKNOWN) {}

TimeRFC868::TimeRFC868(const char* host, uint16_t port = 37) :
    mHost(host),
    mPort(port) {}

secs_t TimeRFC868::getCurrentTime(std::string name) {

    WiFiClient client;
    secs_t currentTime = TIME_INVALID;

    if (0 != client.connect(mHost.c_str(), mPort)) {

        client.printf(name.c_str());
        // client.flush(3000 /* ms */);

        unsigned int wait = 30;
        while((wait--) != 0) {

            if (client.available() >= 4) {

                currentTime = ((secs_t)client.read());
                currentTime |= ((secs_t)client.read()) << 8;
                currentTime |= ((secs_t)client.read()) << 16;
                currentTime |= ((secs_t)client.read()) << 24;
                break;
            }

            delay(100);
        }

        client.stop();
    }

    return currentTime;
}

} // namespace app