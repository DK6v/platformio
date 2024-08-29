#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "network/Network.h"

using namespace app;

Network::Network() {}

Network::Status Network::status() {

    return WiFi.isConnected()
        ? Network::Status::CONNECTED
        : Network::Status::UNDEFINED;
}

Network::Status Network::connect_w_tmo(Network::Type type, uint16_t waitSecs) {

    auto rc = (type == Type::FAST) ? fastConnect() : slowConnect();

    /* Wait for the connection to be established */
    uint16_t maxAttemps = waitSecs * 20;
    while((maxAttemps--) != 0) {
        if (WiFi.isConnected()) {
            rc = Status::CONNECTED;
            break;
        }
        delay(50);
    }

    return rc;
}

Network::Status Network::connect() {

    auto rc = Status::UNDEFINED;
    Config &config = Config::getInstance();

    unsigned long startTimeMs = millis();

#ifdef NW_CONNECT_FAST
    if (config.get<IPAddress>(Config::ID::LOCAL_IP_ADDRESS).isSet()) {
        rc = network.connect_w_tmo(Type::FAST, 8);
    }
#endif
    if (rc != Network::Status::CONNECTED) {
        rc = network.connect_w_tmo(Type::SLOW, 20);
    }

    if (rc == Network::Status::CONNECTED) {
        console.log("Wifi connected in %lu ms", (millis() - startTimeMs));

        uint8_t bssid[6] = { 0 };
        memcpy(bssid, WiFi.BSSID(), sizeof(bssid));
        console.format("AP MAC address (BSSID): %x:%x:%x:%x:%x:%x",
                       bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);

        config.set<std::vector<uint8_t>>(
            Config::ID::WIFI_AP_BSSID,
            std::vector<uint8_t>(bssid, bssid + sizeof(bssid)));

        config.set<char>(
            Config::ID::WIFI_AP_CHANNEL,
            WiFi.channel());

        config.set<IPAddress>(
            Config::ID::LOCAL_IP_ADDRESS,
            WiFi.localIP());

    } else {
        console.log("Wifi connection FAILED in %lu ms", (millis() - startTimeMs));
        console.log("SLOW WiFi: SSID=%s, PSWD=%s",
            config.get<std::string>(Config::ID::WIFI_AP_NAME).c_str(),
            config.get<std::string>(Config::ID::WIFI_AP_PASSWORD).c_str());

        rc = Status::FAILED;
    }

    return rc;
}

Network::Status Network::slowConnect() {

    auto rc = Status::CONNECTING;
    Config &config = Config::getInstance();

    console.log("Network: slow start (SSID=%s, PSWD=%s)",
        config.get<std::string>(Config::ID::WIFI_AP_NAME).c_str(),
        config.get<std::string>(Config::ID::WIFI_AP_PASSWORD).c_str());

    WiFi.begin(
        config.get<std::string>(Config::ID::WIFI_AP_NAME).c_str(),
        config.get<std::string>(Config::ID::WIFI_AP_PASSWORD).c_str());

    return rc;
}

Network::Status Network::fastConnect() {

    auto rc = Status::CONNECTING;
    Config &config = Config::getInstance();

    console.log("Network: set static IP address: %s",
        config.get<IPAddress>(Config::ID::LOCAL_IP_ADDRESS).toString().c_str());

    if (!WiFi.config(config.get<IPAddress>(Config::ID::LOCAL_IP_ADDRESS),
                     config.get<IPAddress>(Config::ID::SETUP_AP_GATEWAY),
                     config.get<IPAddress>(Config::ID::SETUP_AP_NW_MASK))) {

        console.log("Failed to set static IP addresses");
    }

    console.log("Network: fast start (SSID=%s, PSWD=%s, CH=%u, BSSID: %x:%x:%x:%x:%x:%x)",
        config.get<std::string>(Config::ID::WIFI_AP_NAME).c_str(),
        config.get<std::string>(Config::ID::WIFI_AP_PASSWORD).c_str(),
        config.get<char>(Config::ID::WIFI_AP_CHANNEL),
        config.get<std::vector<uint8_t>>(Config::ID::WIFI_AP_BSSID)[0],
        config.get<std::vector<uint8_t>>(Config::ID::WIFI_AP_BSSID)[1],
        config.get<std::vector<uint8_t>>(Config::ID::WIFI_AP_BSSID)[2],
        config.get<std::vector<uint8_t>>(Config::ID::WIFI_AP_BSSID)[3],
        config.get<std::vector<uint8_t>>(Config::ID::WIFI_AP_BSSID)[4],
        config.get<std::vector<uint8_t>>(Config::ID::WIFI_AP_BSSID)[5]);

    WiFi.begin(
        config.get<std::string>(Config::ID::WIFI_AP_NAME).c_str(),
        config.get<std::string>(Config::ID::WIFI_AP_PASSWORD).c_str(),
        config.get<char>(Config::ID::WIFI_AP_CHANNEL),
        config.get<std::vector<uint8_t>>(Config::ID::WIFI_AP_BSSID).data());

    return rc;
}

void Network::disconnect() {

    if(WiFi.isConnected()) {

        WiFi.disconnect(true);
    }
}
