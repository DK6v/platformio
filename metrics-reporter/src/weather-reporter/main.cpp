#include <Arduino.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <AutoConnect.h>

#include "Console.h"
#include "Checksum.h"
#include "Byte.h"
#include "RBufferHelper.h"
#include "WBufferHelper.h"
#include "Timer.h"
#include "TimeRFC868.h"
#include "PinLed.h"
#include "PinBme280.h"
#include "PinBh1750.h"

#include "config/Config.h"
#include "reporter/InfluxReporter.h"
#include "network/Network.h"
#include "server/Controller.h"
#include "server/view/View.h"

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
#define REPORT_ROUND_TIME   900
#endif

using namespace app;

ESP8266WebServer server;
app::Controller controler = app::Controller(&server);

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
    uint16_t restartCounter;
    int16_t batteryVolts;
    int16_t calibration;
    int16_t checksumBits;
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
    Wire.requestFrom(I2C_ADDR, 13);

    auto rbuf = RBufferHelper<TwoWire*>(+[](TwoWire *ptr) -> char {
        return ptr->read();
    }, &Wire);

    for (uint8_t wait = 20; wait != 0; --wait, delay(50))
    {
        if (Wire.available())
        {
            g_pm.restartCounter = rbuf.getBytes(2);
            g_pm.batteryVolts = rbuf.getBytes(2);
            g_pm.calibration = rbuf.getBytes(2);
            g_pm.lastOperationTimeMs = rbuf.getBytes(4);
            g_pm.lastConnectionStatus = static_cast<Network::Status>(rbuf.getByte());
            g_pm.checksumBits = rbuf.getByte();

            checksum = rbuf.getByte();
            break;
        }
    }

    if (checksum != BYTE_XOR(BYTE16(g_pm.restartCounter),
                             BYTE16(g_pm.batteryVolts),
                             BYTE16(g_pm.calibration),
                             BYTE32(g_pm.lastOperationTimeMs),
                             BYTE8(static_cast<uint8_t>(g_pm.lastConnectionStatus)),
                             BYTE8(g_pm.checksumBits)))
    {
        g_pm.restartCounter = 0;
        g_pm.batteryVolts = 0;
        g_pm.calibration = 0;
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

    auto client = WiFiClient();
    auto metric = InfluxReporter(SERVER_ADDRESS, SERVER_PORT);
    metric.create(REPORT_NAME);
    metric.addTag("sensor", SENSOR_NAME);
    metric.addField("temp",
                    ROUNDF(bme.temperature, 2),
                    (!std::isnan(bme.temperature)));
    metric.addField("pressure",
                    ROUNDF(bme.pressure, 2),
                    (!std::isnan(bme.pressure)));
    metric.addField("humidity",
                    ROUNDF(bme.humidity, 2),
                    (!std::isnan(bme.humidity)));
    metric.addField("light",
                    ROUNDF(light.lightLevel, 2),
                    (!std::isnan(light.lightLevel)));
    metric.addField("battery",
                    ROUNDF(g_pm.batteryVolts / 1000.0, 2),
                    (g_pm.batteryVolts != 0));
    metric.addField("rssi",
                    WiFi.RSSI(),
                    WiFi.isConnected());
    metric.addField("calibration",
                    ROUNDF(g_pm.calibration / 100.0, 2),
                    (g_pm.calibration != 0));
    metric.addField("connStartTimeMs",
                    (g_pm.connStartTimeMs - g_pm.powerOnTimeMs),
                    (g_pm.connStartTimeMs != 0));
    metric.addField("connCompleteTimeMs",
                    (g_pm.connCompleteTimeMs - g_pm.powerOnTimeMs),
                    (g_pm.connCompleteTimeMs != 0));
    metric.addField("connStatus",
                    static_cast<uint8_t>(g_pm.connectionStatus),
                    (g_pm.connectionStatus != Network::Status::UNDEFINED));
    metric.addField("lastOperationTimeMs",
                    g_pm.lastOperationTimeMs,
                    (g_pm.lastOperationTimeMs != 0));
    metric.addField("lastConnStatus",
                    static_cast<uint8_t>(g_pm.lastConnectionStatus),
                    (g_pm.lastConnectionStatus != Network::Status::UNDEFINED));
    metric.addField("metricsStartTimeMs",
                    (g_pm.metricsStartTimeMs - g_pm.powerOnTimeMs),
                    (g_pm.metricsStartTimeMs != 0));
    metric.addField("metricsCompleteTimeMs",
                    (g_pm.metricsCompleteTimeMs - g_pm.powerOnTimeMs),
                    (g_pm.metricsCompleteTimeMs != 0));
    metric.addField("sensorsStartTimeMs",
                    (g_pm.sensorsStartTimeMs - g_pm.powerOnTimeMs),
                    (g_pm.sensorsStartTimeMs != 0));
    metric.addField("sensorsCompleteTimeMs",
                    (g_pm.sensorsCompleteTimeMs - g_pm.powerOnTimeMs),
                    (g_pm.sensorsCompleteTimeMs != 0));
    metric.addField("bmeSensorStartTimeMs",
                    (g_pm.bmeSensorStartTimeMs - g_pm.powerOnTimeMs),
                    (g_pm.bmeSensorStartTimeMs != 0));
    metric.addField("bmeSensorCompleteTimeMs",
                    (g_pm.bmeSensorCompleteTimeMs - g_pm.powerOnTimeMs),
                    (g_pm.bmeSensorCompleteTimeMs != 0));
    metric.addField("lightSensorStartTimeMs",
                    (g_pm.lightSensorStartTimeMs - g_pm.powerOnTimeMs),
                    (g_pm.lightSensorStartTimeMs != 0));
    metric.addField("lightSensorCompleteTimeMs",
                    (g_pm.lightSensorCompleteTimeMs - g_pm.powerOnTimeMs),
                    (g_pm.lightSensorCompleteTimeMs != 0));
    metric.addField("timeToReportMs",
                    (millis() - g_pm.powerOnTimeMs),
                    (g_pm.powerOnTimeMs != 0));
    metric.addField("checksumBits",
                    g_pm.checksumBits,
                    (g_pm.checksumBits != 0xFF));
    metric.addField("restartCounter",
                    g_pm.restartCounter);

#ifdef BUILD_DEBUG
    metric.addField("debug", "true");
#endif

    if (currentTime != TIME_INVALID) {
        metric.send(client, currentTime);
    } else {
        metric.send(client);
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
        .add<std::string>(Config::ID::WIFI_AP_NAME, WIFI_NAME)
        .add<std::string>(Config::ID::WIFI_AP_PASSWORD, WIFI_PASSWORD)
        .add<std::string>(Config::ID::SETUP_AP_NAME, "ESP8266")
        .add<std::string>(Config::ID::SETUP_AP_PASSWORD, "12345678")
        .add<IPAddress>(Config::ID::SETUP_AP_ADDRESS, IPAddress(192, 168, 0, 1))
        .add<IPAddress>(Config::ID::SETUP_AP_GATEWAY, IPAddress(192, 168, 0, 1))
        .add<IPAddress>(Config::ID::SETUP_AP_NW_MASK, IPAddress(255, 255, 255, 0))
        .add<std::vector<uint8_t>>(Config::ID::WIFI_AP_BSSID, {})
        .add<char>(Config::ID::WIFI_AP_CHANNEL, 0xFF)
        .add<IPAddress>(Config::ID::LOCAL_IP_ADDRESS, IPAddress())
        .read(eeprom);

    readPowerBoardCounters();

    g_pm.connStartTimeMs = millis();
    g_pm.connectionStatus = Network::Status::DISCONNECTED;

#ifdef NW_CONNECT_FAST
    if (g_pm.restartCounter == 0) {
        g_pm.connectionStatus = network.connect(Network::Type::SLOW);
    } else {
        g_pm.connectionStatus = network.connect(Network::Type::FAST);
    }
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

        currentTime = timeRfc868.epoch(REPORT_NAME);
        currentTicks = millis();

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