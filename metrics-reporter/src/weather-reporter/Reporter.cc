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

bool Reporter::send(std::string metric) {

    WiFiClient client;

    if (client.connect(mHost.c_str(), mPort)) {

        client.printf("%s\n", metric.c_str());
        return client.stop(0);
    }

    return false;
}

bool Reporter::send(const char* metric) {
    return send(std::string(metric));
}

} // namespace app