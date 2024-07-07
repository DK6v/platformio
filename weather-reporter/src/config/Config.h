#pragma once

#include <map>
#include <memory>

#include <Arduino.h>
#include "Console.h"

#include "ConfigParameter.h"
#include "ByteBuffer.h"
#include "StorageEeprom.h"
#include "../common/Checksum.h"

namespace app {

class Config {
private:
    Config();
    ~Config() = default;

    Config(Config const&) = delete;
    Config& operator= (Config const&) = delete;

public:
    static Config& getInstance() {
        static Config config;
        return config;
    }

    template<typename T>
    Config& add(ConfigParameter<T>* parameter_p);

    template<typename T>
    Config& add(unsigned char id, const T &value);

    template<typename T>
    T get(uint8_t id);

    template<typename T>
    bool set(uint8_t id, const T& value);

    Config& read(ByteBuffer& buffer);
    Config& write(ByteBuffer& buffer);

public:
    enum ID : unsigned char;

private:
    std::map<uint8_t, std::shared_ptr<ConfigParameterBase>> mParameters;
};

enum Config::ID : unsigned char {
    UNDEFINED = 0,
    WIFI_AP_NAME,
    WIFI_AP_PASSWORD,
    SETUP_AP_NAME,
    SETUP_AP_PASSWORD,
    SETUP_AP_ADDRESS,
    SETUP_AP_GATEWAY,
    SETUP_AP_NW_MASK,
};

template<typename T>
Config& Config::add(ConfigParameter<T>* parameter_p) {

    mParameters.insert(std::pair<uint8_t, std::shared_ptr<ConfigParameterBase>>(
        parameter_p->getId(),
        std::make_shared<ConfigParameter<T>>(*parameter_p)));

    return *this;
}

template<typename T>
Config& Config::add(unsigned char id, const T &value) {

    add<T>(new ConfigParameter<T>(id, value));
    return *this;
}

template<typename T>
T Config::get(uint8_t id) {

    for (auto & [ _, parameter]: mParameters) {

        if (parameter->getId() == id) {

            return *std::static_pointer_cast<ConfigParameter<T>>(parameter);
        }
    }

    return *std::shared_ptr<ConfigParameter<T>>(new ConfigParameter<T>());
}

template<typename T>
bool Config::set(uint8_t id, const T& value) {

    for (auto & [ _, parameter]: mParameters) {

        if (parameter->getId() == id) {

            *std::static_pointer_cast<ConfigParameter<T>>(parameter) = value;
            return true;
        }
    }

    return false;
}

}