#include <list>
#include <array>
#include <string>
#include <sstream>

#include <Arduino.h>
#include <WiFiClient.h>

#include "TimeRFC868.h"

namespace app {

TimeRFC868::TimeRFC868(const std::string& host, uint16_t port = 37) :
    mHost(host),
    mPort(port),
    mStatus(Status::UNKNOWN) {}

TimeRFC868::TimeRFC868(const char* host, uint16_t port = 37) :
    mHost(host),
    mPort(port) {}

unsigned int TimeRFC868::getCurrentTime() {

    WiFiClient client;
    unsigned int currentTime = 0;


    if (0 != client.connect(mHost.c_str(), mPort)) {
        
        unsigned int wait = 30;
        while((wait--) != 0) {   

            if (client.available() >= 4) {

                currentTime += ((uint8_t)client.read());
                currentTime += ((uint8_t)client.read()) << 8;
                currentTime += ((uint8_t)client.read()) << 16;
                currentTime += ((uint8_t)client.read()) << 24;
                break;
            }

            delay(100);
        }

        client.stop();
    }

    return currentTime;
}

} // namespace app