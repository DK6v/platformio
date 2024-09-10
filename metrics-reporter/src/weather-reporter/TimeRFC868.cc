#include <list>
#include <array>
#include <string>
#include <sstream>

#include <Arduino.h>
#include <WiFiClient.h>

#include "Timer.h"
#include "TimeRFC868.h"
#include "RBufferHelper.h"

namespace app {

TimeRFC868::TimeRFC868(const std::string& host, uint16_t port = 37) :
    mHost(host),
    mPort(port),
    mStatus(Status::UNKNOWN) {}

TimeRFC868::TimeRFC868(const char* host, uint16_t port = 37) :
    mHost(host),
    mPort(port) {}

secs_t TimeRFC868::datetime(std::string name) {

    WiFiClient client;
    secs_t currentTime = TIME_INVALID;

    if (client.connect(mHost.c_str(), mPort)) {

        auto rbuf = RBufferHelper<WiFiClient*>(+[](WiFiClient *ptr) -> char {
            return ptr->read();
        }, &client);

        client.setSync(true);
        client.write(name.c_str());

        unsigned int wait = 300;
        while((wait--) != 0) {
            delay(10);
            if (client.available() >= 4) {
                currentTime = static_cast<secs_t>(rbuf.getBytes(4));
                break;
            }
        }

        client.stop();
    }

    return currentTime;
}

secs_t TimeRFC868::epoch(std::string name) {

    secs_t currentTime = datetime(name);

    if (currentTime != TIME_INVALID) {

        // Convert to unix epoch (1900 -> 1970)
        currentTime -= 2208988800;

        if ((currentTime < 1577836800 /* 2020 */) ||
            (currentTime > 2524608000 /* 2050 */)) {

            return TIME_INVALID;
        }

        return currentTime;
    }

    return TIME_INVALID;
}

} // namespace app