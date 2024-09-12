#include <list>
#include <string>
#include <tuple>

#include <Arduino.h>
#include <Client.h>

#include "Reporter.h"

namespace app {

Reporter::Reporter(const std::string& host, uint16_t port)
: mHost(host),
  mPort(port)
{}

void Reporter::clear() {
    mMetric.clear();
    mTags.clear();
    mFields.clear();
}

void Reporter::create(const std::string &metric) {
    mMetric = metric;
}

void Reporter::addTag(const std::string &tag,
                           const std::string &value,
                           bool condition) {
    if (condition) {
        mTags.push_back(std::make_tuple(tag, value));
    }
}

void Reporter::addField(const std::string &tag,
                             const std::string &value,
                             bool condition) {
    if (condition) {
        mFields.push_back(std::make_tuple(tag, value));
    }
}

} // namespace app