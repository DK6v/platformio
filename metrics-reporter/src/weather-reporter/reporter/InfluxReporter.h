#pragma once

#include "Arduino.h"
#include "Client.h"

#include <reporter/Reporter.h>

namespace app {

class InfluxReporter : public Reporter {
public:
    InfluxReporter(const std::string &host, uint16_t port);

    virtual size_t send(Client &client, uint32_t timestamp = 0) override;
};

} // namespace fm