#include <list>
#include <array>
#include <string>
#include <sstream>

#include <Arduino.h>
#include <WiFiClient.h>

#include "Reporter.h"

namespace app {

Reporter::Reporter(const std::string& host, uint16_t port) :
    mHost(host),
    mPort(port),
    mStatus(Status::UNKNOWN) {}

Reporter::Reporter(const char* host, uint16_t port) :
    mHost(host),
    mPort(port) {}

size_t Reporter::send(std::string metric) {

    WiFiClient client;
    size_t bytesSent = 0;

    client.setDefaultSync(true);

    if (0 != client.connect(mHost.c_str(), mPort)) {
        
        bytesSent = client.printf("%s\n", metric.c_str());
        client.flush(3000 /* ms */);

        delay(100);
        client.stop();
    }

    return bytesSent;
}

size_t Reporter::send(const char* metric) {
    return send(std::string(metric));
}

} // namespace app