#include <list>
#include <string>
#include <tuple>

#include <Arduino.h>
#include <Client.h>

#include "reporter/Reporter.h"
#include "reporter/InfluxReporter.h"

namespace app {

InfluxReporter::InfluxReporter(const std::string &host, uint16_t port)
: Reporter(host, port) {}

size_t InfluxReporter::send(Client &client, uint32_t timestamp) {

    if (mMetric.empty() || (mTags.empty() && mFields.empty())) {

        return 0;
    }

    if (client.connect(mHost.c_str(), mPort)) {

        std::string message = mMetric;

        for (const auto &tag: mTags) {
            message += "," + std::get<NAME>(tag) + "=" + std::get<VALUE>(tag);
        }

        if (mFields.size()) {
            message += " ";
        }

        for (const auto & field: mFields) {
            if (&field != &(*mFields.begin())) {
                message += ",";
            }
            message += std::get<NAME>(field) + "=" + std::get<VALUE>(field);
        }

        if (timestamp != 0) {
            message += " ";
            message += std::to_string(timestamp) + "000000000" /* ns */;
        }

        message += "\n";

        auto size = client.write(
            reinterpret_cast<const unsigned char*>(message.c_str()),
            message.length() + 1);

        client.flush();
        client.stop();

        return size;
    }

    return 0;
}

} // namespace app