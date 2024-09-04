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

secs_t TimeRFC868::getCurrentTime(std::string name) {

    WiFiClient client;
    secs_t currentTime = TIME_INVALID;

    if (client.connect(mHost.c_str(), mPort)) {

        auto rbuf = RBufferHelper<WiFiClient*>(+[](WiFiClient *ptr) -> char {
            return ptr->read();
        }, &client);

        client.setSync(true);
        client.write(name.c_str());
        client.flush(3 * SECONDS);

        unsigned int wait = 100;
        while((wait--) != 0) {

            if (client.available() >= 4) {
                currentTime = static_cast<secs_t>(rbuf.getBytes(4));
                break;
            }

            delay(100);
        }

        client.stop();
    }

    return currentTime;
}

} // namespace app