#include <Arduino.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <AutoConnect.h>

#include "Console.h"
#include "Checksum.h"
#include "config/Config.h"
#include "Byte.h"
#include "RBufferHelper.h"
#include "WBufferHelper.h"
#include "server/Controller.h"
#include "server/view/View.h"
#include "network/Network.h"
#include "Timer.h"
#include "TimeRFC868.h"
#include "Reporter.h"
#include "PinLed.h"
#include "PinBme280.h"
#include "PinBh1750.h"

#define I2C_ADDR 0x0A

#ifdef BUILD_DEBUG
#define REPORT_NAME         "test"
#define SENSOR_NAME         "test"
#define REPORT_INTERVAL     30
#define REPORT_ROUND_TIME   30
#else
#define REPORT_NAME         "weather"
#define SENSOR_NAME         "bme280"
#define REPORT_INTERVAL     900
#define REPORT_ROUND_TIME   300
#endif

using namespace app;

ESP8266WebServer server;
app::Controller controler = app::Controller(&server);

app::Reporter reporter("192.168.0.5", 42001);
app::TimeRFC868 timeRfc868("192.168.0.5", 37);
app::PinBme280 bme(PIN_SDA, PIN_SCL);
app::PinBH1750 light(PIN_SDA, PIN_SCL);

app::PinLed pinLed(PIN_LED, true);

volatile secs_t currentTime = TIME_INVALID;
volatile ticks_t currentTicks = TICKS_INVALID;

volatile secs_t sleepInterval = REPORT_INTERVAL;

// unsigned long startTimeMs = millis();

volatile struct PmCounters
{
    // Power board metrics
    int16_t batteryVolts;
    int16_t calibration;
    int16_t checksumBits;
    uint32_t boardTime;
    uint32_t lastOperationTimeMs;
    Network::Status lastConnectionStatus;

    // Operational metrics
    uint32_t powerOnTimeMs;
    uint32_t connStartTimeMs;
    uint32_t connCompleteTimeMs;
    uint32_t metricsStartTimeMs;
    uint32_t metricsCompleteTimeMs;
    uint32_t sensorsStartTimeMs;
    uint32_t sensorsCompleteTimeMs;
    uint32_t bmeSensorStartTimeMs;
    uint32_t bmeSensorCompleteTimeMs;
    uint32_t lightSensorStartTimeMs;
    uint32_t lightSensorCompleteTimeMs;

    // Results
    Network::Status connectionStatus;
} g_pm;

void readPowerBoardCounters()
{
    char checksum = 0xFF;

    g_pm.metricsStartTimeMs = millis();

    Wire.begin(PIN_SDA, PIN_SCL);
    Wire.beginTransmission(I2C_ADDR);
    Wire.requestFrom(I2C_ADDR, 15);

    auto rbuf = RBufferHelper<TwoWire*>(+[](TwoWire *ptr) -> char {
        return ptr->read();
    }, &Wire);

    for (uint8_t wait = 20; wait != 0; --wait, delay(50))
    {
        if (Wire.available())
        {
            g_pm.batteryVolts = rbuf.getBytes(2);
            g_pm.calibration = rbuf.getBytes(2);
            g_pm.boardTime = rbuf.getBytes(4);
            g_pm.lastOperationTimeMs = rbuf.getBytes(4);
            g_pm.lastConnectionStatus = static_cast<Network::Status>(rbuf.getByte());
            g_pm.checksumBits = rbuf.getByte();

            checksum = rbuf.getByte();

            break;
        }
    }

    if (checksum != BYTE_XOR(BYTE16(g_pm.batteryVolts),
                             BYTE16(g_pm.calibration),
                             BYTE32(g_pm.boardTime),
                             BYTE32(g_pm.lastOperationTimeMs),
                             BYTE8(static_cast<uint8_t>(g_pm.lastConnectionStatus)),
                             BYTE8(g_pm.checksumBits)))
    {
        g_pm.batteryVolts = 0;
        g_pm.calibration = 0;
        g_pm.boardTime = 0;
        g_pm.lastOperationTimeMs = 0;
        g_pm.lastConnectionStatus = Network::Status::UNDEFINED;
        g_pm.checksumBits = 0;
    }

    Wire.endTransmission();

    g_pm.metricsCompleteTimeMs = millis();
}

void sendReport()
{
    std::string fields;

    readPowerBoardCounters();

    g_pm.sensorsStartTimeMs = millis();
    g_pm.bmeSensorStartTimeMs = millis();

    if (bme.begin(0x76)) {
        bme.read();
    }

    g_pm.bmeSensorCompleteTimeMs = millis();
    g_pm.lightSensorStartTimeMs = millis();

    if (light.begin(0x23)) {
        light.read();
    }

    g_pm.lightSensorCompleteTimeMs = millis();
    g_pm.sensorsCompleteTimeMs = millis();

    if (!std::isnan(bme.temperature))
    {
        fields += (fields.length() != 0) ? "," : "";
        fields += "temp=";
        fields += std::to_string(ROUNDF(bme.temperature, 2));
    }

    if (!std::isnan(bme.pressure))
    {
        fields += (fields.length() != 0) ? "," : "";
        fields += "pressure=";
        fields += std::to_string(ROUNDF(bme.pressure, 2));
    }

    if (!std::isnan(bme.humidity))
    {
        fields += (fields.length() != 0) ? "," : "";
        fields += "humidity=";
        fields += std::to_string(ROUNDF(bme.humidity, 2));
    }

    if (!std::isnan(light.lightLevel))
    {
        fields += (fields.length() != 0) ? "," : "";
        fields += "light=";
        fields += std::to_string(ROUNDF(light.lightLevel, 2));
    }

    if (g_pm.batteryVolts != 0)
    {
        fields += (fields.length() != 0) ? "," : "";
        fields += "battery=";
        fields += std::to_string(ROUNDF(g_pm.batteryVolts / 1000.0, 2));
    }

    if (WiFi.isConnected())
    {
        fields += (fields.length() != 0) ? "," : "";
        fields += "rssi=";
        fields += std::to_string(WiFi.RSSI());
    }

    if (g_pm.calibration != 0)
    {
        fields += (fields.length() != 0) ? "," : "";
        fields += "calibration=";
        fields += std::to_string(g_pm.calibration / 100.0);
    }

    if (g_pm.lastOperationTimeMs != 0)
    {
        fields += (fields.length() != 0) ? "," : "";
        fields += "lastOperationTimeMs=";
        fields += std::to_string(g_pm.lastOperationTimeMs);
    }

    if (g_pm.lastConnectionStatus != Network::Status::UNDEFINED)
    {
        fields += (fields.length() != 0) ? "," : "";
        fields += "lastConnStatus=";
        fields += std::to_string(static_cast<uint8_t>(g_pm.lastConnectionStatus));
    }

    if (g_pm.connStartTimeMs != 0)
    {
        fields += (fields.length() != 0) ? "," : "";
        fields += "connStartTimeMs=";
        fields += std::to_string(g_pm.connStartTimeMs - g_pm.powerOnTimeMs);
    }

    if (g_pm.connCompleteTimeMs != 0)
    {
        fields += (fields.length() != 0) ? "," : "";
        fields += "connCompleteTimeMs=";
        fields += std::to_string(g_pm.connCompleteTimeMs - g_pm.powerOnTimeMs);
    }

    if (g_pm.connectionStatus != Network::Status::UNDEFINED)
    {
        fields += (fields.length() != 0) ? "," : "";
        fields += "connStatus=";
        fields += std::to_string(static_cast<uint8_t>(g_pm.connectionStatus));
    }

    if (g_pm.metricsStartTimeMs != 0)
    {
        fields += (fields.length() != 0) ? "," : "";
        fields += "metricsStartTimeMs=";
        fields += std::to_string(g_pm.metricsStartTimeMs - g_pm.powerOnTimeMs);
    }

    if (g_pm.metricsCompleteTimeMs != 0)
    {
        fields += (fields.length() != 0) ? "," : "";
        fields += "metricsCompleteTimeMs=";
        fields += std::to_string(g_pm.metricsCompleteTimeMs - g_pm.powerOnTimeMs);
    }

    if (g_pm.sensorsStartTimeMs != 0)
    {
        fields += (fields.length() != 0) ? "," : "";
        fields += "sensorsStartTimeMs=";
        fields += std::to_string(g_pm.sensorsStartTimeMs - g_pm.powerOnTimeMs);
    }

    if (g_pm.sensorsCompleteTimeMs != 0)
    {
        fields += (fields.length() != 0) ? "," : "";
        fields += "sensorsCompleteTimeMs=";
        fields += std::to_string(g_pm.sensorsCompleteTimeMs - g_pm.powerOnTimeMs);
    }

    if (g_pm.bmeSensorStartTimeMs != 0)
    {
        fields += (fields.length() != 0) ? "," : "";
        fields += "bmeSensorStartTimeMs=";
        fields += std::to_string(g_pm.bmeSensorStartTimeMs - g_pm.powerOnTimeMs);
    }

    if (g_pm.bmeSensorCompleteTimeMs != 0)
    {
        fields += (fields.length() != 0) ? "," : "";
        fields += "bmeSensorCompleteTimeMs=";
        fields += std::to_string(g_pm.bmeSensorCompleteTimeMs - g_pm.powerOnTimeMs);
    }

    if (g_pm.lightSensorStartTimeMs != 0)
    {
        fields += (fields.length() != 0) ? "," : "";
        fields += "lightSensorStartTimeMs=";
        fields += std::to_string(g_pm.lightSensorStartTimeMs - g_pm.powerOnTimeMs);
    }

    if (g_pm.lightSensorCompleteTimeMs != 0)
    {
        fields += (fields.length() != 0) ? "," : "";
        fields += "lightSensorCompleteTimeMs=";
        fields += std::to_string(g_pm.lightSensorCompleteTimeMs - g_pm.powerOnTimeMs);
    }

    if (g_pm.powerOnTimeMs != 0)
    {
        fields += (fields.length() != 0) ? "," : "";
        fields += "timeToReportMs=";
        fields += std::to_string(millis() - g_pm.powerOnTimeMs);
    }

    if (g_pm.checksumBits != 0xFF)
    {
        fields += (fields.length() != 0) ? "," : "";
        fields += "checksumBits=";
        fields += std::to_string(g_pm.checksumBits);
    }

#ifdef BUILD_DEBUG
    if (g_pm.boardTime != 0)
    {
        fields += (fields.length() != 0) ? "," : "";
        fields += "boardTime=";
        fields += std::to_string(g_pm.boardTime);
    }

    fields += (fields.length() != 0) ? "," : "";
    fields += "debug=true";
#endif

    if (WiFi.isConnected() && !fields.empty())
    {
        std::string header = std::string(REPORT_NAME ",sensor=" SENSOR_NAME " ");

        if (currentTime != TIME_INVALID)
        {
            fields += (fields.length() != 0) ? " " : "";
            fields += std::to_string(currentTime) + "000000000" /* ns */;
        }

        console.log("Send: %s", (header + fields).c_str());
        reporter.send(header + fields);
    }
}

void setup()
{
    app::console.log("cplusplus:%u", __cplusplus);
    app::console.log("=> Start <=");

    memset(const_cast<PmCounters *>(&g_pm), 0, sizeof(PmCounters));
    g_pm.powerOnTimeMs = millis();

    app::StorageEeprom eeprom = app::StorageEeprom(128);

    Config &config = Config::getInstance()
        .add<std::string>(Config::ID::WIFI_AP_NAME, "elbrus-wlan")
        .add<std::string>(Config::ID::WIFI_AP_PASSWORD, "Peppa@Pig")
        .add<std::string>(Config::ID::SETUP_AP_NAME, "ESP8266")
        .add<std::string>(Config::ID::SETUP_AP_PASSWORD, "12345678")
        .add<IPAddress>(Config::ID::SETUP_AP_ADDRESS, IPAddress(192, 168, 0, 1))
        .add<IPAddress>(Config::ID::SETUP_AP_GATEWAY, IPAddress(192, 168, 0, 1))
        .add<IPAddress>(Config::ID::SETUP_AP_NW_MASK, IPAddress(255, 255, 255, 0))
        .add<std::vector<uint8_t>>(Config::ID::WIFI_AP_BSSID, {})
        .add<char>(Config::ID::WIFI_AP_CHANNEL, 0xFF)
        .add<IPAddress>(Config::ID::LOCAL_IP_ADDRESS, IPAddress())
        .read(eeprom);

    g_pm.connStartTimeMs = millis();
    g_pm.connectionStatus = Network::Status::DISCONNECTED;

#ifdef NW_CONNECT_FAST
    g_pm.connectionStatus = network.connect();
#else
    WiFi.begin(
        config.get<std::string>(Config::ID::WIFI_AP_NAME).c_str(),
        config.get<std::string>(Config::ID::WIFI_AP_PASSWORD).c_str());

    /* Wait for the connection to be established */
    for (int maxAttemps = 100; maxAttemps > 0; --maxAttemps)
    {
        if (WiFi.isConnected())
            break;

        delay(100);
    }
#endif

    g_pm.connCompleteTimeMs = millis();

    if (network.status() == Network::Status::CONNECTED)
    {

        console.log("Connected SSID '%s', channel %u",
                    config.get<std::string>(Config::ID::WIFI_AP_NAME).c_str(),
                    WiFi.channel());

        console.log("IP address: %s",
                    WiFi.localIP().toString().c_str());

        currentTime = timeRfc868.getCurrentTime(REPORT_NAME);
        currentTicks = millis();

        if (currentTime != TIME_INVALID)
        {
            currentTime -= 2208988800; // Convert to unix epoch (1900 -> 1970)
        }

        console.log("Current time: %llu, ticks: %lu", currentTime, currentTicks);

        sendReport();

        network.disconnect();
    }
    else
    {
        console.log("Can't connect SSID %s, status=%d", WiFi.status());

        WiFi.mode(WIFI_AP_STA);

        WiFi.softAPConfig(
            config.get<IPAddress>(Config::ID::SETUP_AP_ADDRESS),
            config.get<IPAddress>(Config::ID::SETUP_AP_GATEWAY),
            config.get<IPAddress>(Config::ID::SETUP_AP_NW_MASK));

        int rc = WiFi.softAP(
            config.get<std::string>(Config::ID::SETUP_AP_NAME).c_str(),
            config.get<std::string>(Config::ID::SETUP_AP_PASSWORD).c_str());

        console.log("Create WiFi AP: %s", (rc == false) ? "failed" : "OK");
        console.log("IP address: %s", WiFi.softAPIP().toString().c_str());

        if (rc == true)
        {
            auto rootUrl = std::shared_ptr<Route>(new Route);

            HtmlView configView = HtmlView();
            configView.add(BodyView().add(TextInputView()));

            rootUrl->setView(configView);

            controler.addRouteHandler("/", rootUrl);

            server.begin();
        }

        //    controler.handleRequest();
    }

    config.write(eeprom);
}

void loop()
{
    Wire.begin(PIN_SDA, PIN_SCL);
    auto wbuf = WBufferHelper<TwoWire*>(+[](char value, TwoWire *ptr) -> void {
        ptr->write(value);
    }, &Wire);

    if (currentTime != TIME_INVALID)
    {
        Wire.beginTransmission(I2C_ADDR);

        app::secs_t delta = (app::secs_t)(millis() - currentTicks) / app::SECONDS;
        currentTime += RANGE(delta, 0, 120);

        wbuf.setByte(I2C_ADDR);
        wbuf.setByte('T');
        wbuf.setBytes(4, currentTime);
        wbuf.setByte(BYTE_XOR('T', BYTE32(currentTime)));

        Wire.endTransmission();
    }

    if (g_pm.connectionStatus != Network::Status::UNDEFINED)
    {
        Wire.beginTransmission(I2C_ADDR);

        wbuf.setBytes(1, I2C_ADDR);
        wbuf.setBytes(1, 'R');
        wbuf.setBytes(1, static_cast<char>(g_pm.connectionStatus));
        wbuf.setBytes(1, BYTE_XOR('R', BYTE8(static_cast<char>(g_pm.connectionStatus))));

        Wire.endTransmission();
    }

    while (true)
    {
        Wire.beginTransmission(I2C_ADDR);

        wbuf.setBytes(1, I2C_ADDR);
        wbuf.setBytes(1, 'S');
        wbuf.setBit(true);
        wbuf.setBits(15, REPORT_INTERVAL);
        wbuf.setBit(true);
        wbuf.setBits(15, REPORT_ROUND_TIME);
        wbuf.setBytes(1, BYTE_XOR('S', BYTE_XOR(BYTE16(REPORT_INTERVAL),
                                                BYTE16(REPORT_ROUND_TIME))));

        Wire.endTransmission();

        delay(3 * app::SECONDS);
    }
}

#if 0
void handleForm() {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
  } else {
    String message = "POST form was:\n";
    for (unsigned int ix = 0; ix < server.args(); ++ix) {
      console.log("arg[%d]: name=%s value=%s",
                  ix, server.argName(ix), server.arg(ix).c_str());
    }
    server.send(200, "text/plain", message);
  }
}
#endif