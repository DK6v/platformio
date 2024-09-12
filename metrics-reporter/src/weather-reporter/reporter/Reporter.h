#pragma once

#include <list>
#include <string>

#include <Arduino.h>
#include "Client.h"

namespace app {

class Reporter {
public:
    Reporter(const std::string& host, uint16_t port);
    virtual ~Reporter() = default;

    virtual void clear();

    virtual void create(const std::string &metric);

    virtual void addTag(const std::string &tag,
                     const std::string &value,
                     bool condition = true);

    virtual void addField(const std::string &tag,
                       const std::string &value,
                       bool condition = true);

    template<typename T,
             typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
    void addTag(const std::string &tag,
                     const T &value,
                     bool condition = true);

    template<typename T,
             typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
    void addField(const std::string &tag,
                       const T &value,
                       bool condition = true);

    virtual size_t send(Client &client, uint32_t timestamp = 0) = 0;

protected:
    using Metric = std::tuple<std::string, std::string>;
    enum MetricFields { NAME, VALUE };

    std::string mHost;
    uint16_t mPort;

    std::string mMetric;
    std::list<Metric> mTags = {};
    std::list<Metric> mFields = {};
};

template<typename T,
         typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
void Reporter::addTag(const std::string &tag, const T &value, bool condition) {
    if (condition) {
        mTags.push_back(std::make_tuple(tag, std::to_string(value)));
    }
}

template<typename T,
         typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
void Reporter::addField(const std::string &tag, const T &value, bool condition) {
    if (condition) {
        mFields.push_back(std::make_tuple(tag, std::to_string(value)));
    }
}

} // namespace fm