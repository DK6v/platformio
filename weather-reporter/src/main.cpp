#include <Arduino.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <AutoConnect.h>

#include "Console.h"
#include "common/Checksum.h"
#include "config/Config.h"
#include "common/Byte.h"
#include "server/Controller.h"
#include "server/view/View.h"
#include "Timer.h"
#include "TimeRFC868.h"
#include "Reporter.h"
#include "PinLed.h"
#include "PinBme280.h"
#include "PinBh1750.h"

#define I2C_ADDR 0x0A

#ifdef BUILD_DEBUG
#define REPORT_NAME "test"
#define SENSOR_NAME "test"
#define REPORT_INTERVAL 30
#else
#define REPORT_NAME "weather"
#define SENSOR_NAME "bme280"
#define REPORT_INTERVAL 900
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

// void handleForm() {
//   if (server.method() != HTTP_POST) {
//     server.send(405, "text/plain", "Method Not Allowed");
//   } else {
//     String message = "POST form was:\n";
//
//     for (unsigned int ix = 0; ix < server.args(); ++ix) {
//       console.log("arg[%d]: name=%s value=%s",
//                   ix, server.argName(ix), server.arg(ix).c_str());
//     }
//
//     server.send(200, "text/plain", message);
//   }
// }

void sendReport()
{
    int16_t batteryVolts = 0;
    int16_t calibration = 0;
    int16_t checksumBits = 0xFF;
    uint32_t boardTime = 0;
    char checksum = 0xFF;

    std::string fields;

    Wire.begin(PIN_SDA, PIN_SCL);

    // Read battery voltage

    Wire.beginTransmission(I2C_ADDR);
    Wire.requestFrom(I2C_ADDR, 10);

    for (uint8_t wait = 30; wait != 0; --wait, delay(100))
    {
        if (Wire.available())
        {
            batteryVolts = ((uint16_t)Wire.read());
            batteryVolts |= ((uint16_t)Wire.read()) << 8;

            calibration = (int16_t)Wire.read();
            calibration |= ((int16_t)Wire.read()) << 8;

            boardTime = ((uint32_t)Wire.read());
            boardTime |= ((uint32_t)Wire.read()) << 8;
            boardTime |= ((uint32_t)Wire.read()) << 16;
            boardTime |= ((uint32_t)Wire.read()) << 24;

            checksumBits = (uint8_t)Wire.read();

            /* Skip checksum */
            checksum = Wire.read();

            break;
        }
    }

    if (checksum == BYTE_XOR(BYTE16(batteryVolts),
                             BYTE16(calibration),
                             BYTE32(boardTime),
                             BYTE8(checksumBits)))
    {
        checksumBits |= 0x04;
    }
    else
    {
        batteryVolts = 0;
        calibration = 0;
        checksumBits = 0;
    }

    Wire.endTransmission();

    // Read measurements

    for (uint8_t wait = 5; wait != 0; --wait, delay(200)) {

        Wire.beginTransmission(0x76);
        uint8_t rc = Wire.endTransmission();
    
        if (rc == 0) {
            bme.begin(0x76);
            bme.read();
            break;
        }
    }

    for (uint8_t wait = 5; wait != 0; --wait, delay(200)) {

        Wire.beginTransmission(0x23);
        uint8_t rc = Wire.endTransmission();
        if (rc == 0) {
            light.begin(0x23);
            light.read();
            break;
        }
    }

    if (!std::isnan(bme.temperature))
    {
        fields += (fields.length() != 0) ? "," : "";
        fields += "temp=" + std::to_string(ROUNDF(bme.temperature, 2));
    }

    if (!std::isnan(bme.pressure))
    {
        fields += (fields.length() != 0) ? "," : "";
        fields += "pressure=" + std::to_string(ROUNDF(bme.pressure, 2));
    }

    if (!std::isnan(bme.humidity))
    {
        fields += (fields.length() != 0) ? "," : "";
        fields += "humidity=" + std::to_string(ROUNDF(bme.humidity, 2));
    }

    if (!std::isnan(light.lightLevel))
    {
        fields += (fields.length() != 0) ? "," : "";
        fields += "light=" + std::to_string(ROUNDF(light.lightLevel, 2));
    }

    if (batteryVolts != 0)
    {
        fields += (fields.length() != 0) ? "," : "";
        fields += "battery=" + std::to_string(ROUNDF(batteryVolts / 1000.0, 2));
    }

    if (WiFi.isConnected())
    {
        fields += (fields.length() != 0) ? "," : "";
        fields += "rssi=" + std::to_string(WiFi.RSSI());
    }

    if (calibration != 0)
    {
        fields += (fields.length() != 0) ? "," : "";
        fields += "calibration=" + std::to_string(calibration / 100.0);
    }

    if (checksumBits != 0xFF)
    {
        fields += (fields.length() != 0) ? "," : "";
        fields += "checksumBits=" + std::to_string(checksumBits);
    }

#ifdef BUILD_DEBUG
    if (boardTime != 0)
    {
        fields += (fields.length() != 0) ? "," : "";
        fields += "boardTime=" + std::to_string(boardTime);
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
    pinLed.blink(PinLed::LONG, 2);

    app::console.log("cplusplus:%u", __cplusplus);
    app::console.log("=> Start <=");

    app::StorageEeprom eeprom = app::StorageEeprom(128);

    Config &config = Config::getInstance()
                         .add<std::string>(Config::ID::WIFI_AP_NAME, "elbrus-wlan")
                         .add<std::string>(Config::ID::WIFI_AP_PASSWORD, "Peppa@Pig")
                         .add<std::string>(Config::ID::SETUP_AP_NAME, "ESP8266")
                         .add<std::string>(Config::ID::SETUP_AP_PASSWORD, "12345678")
                         .add<IPAddress>(Config::ID::SETUP_AP_ADDRESS, IPAddress(192, 168, 0, 1))
                         .add<IPAddress>(Config::ID::SETUP_AP_GATEWAY, IPAddress(192, 168, 0, 1))
                         .add<IPAddress>(Config::ID::SETUP_AP_NW_MASK, IPAddress(255, 255, 255, 0))
                         .read(eeprom);

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

    if (WiFi.isConnected())
    {
        console.log("Connected SSID '%s'",
                    config.get<std::string>(Config::ID::WIFI_AP_NAME).c_str());
        console.log("IP address: %s", WiFi.localIP().toString().c_str());

        currentTime = timeRfc868.getCurrentTime(REPORT_NAME);
        currentTicks = millis();

        if (currentTime != TIME_INVALID)
        {
            currentTime -= 2208988800; // Convert to unix epoch (1900 -> 1970)
        }

        console.log("Current time: %llu, ticks: %lu", currentTime, currentTicks);

        sendReport();

        pinLed.blink(app::PinLed::NORM);
        WiFi.disconnect(true);
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
            configView.add(BodyView()
                               .add(TextInputView()));

            rootUrl->setView(configView);

            controler.addRouteHandler("/", rootUrl);

            server.begin();
        }

        //    controler.handleRequest();
    }
}

void loop()
{
    Wire.begin(PIN_SDA, PIN_SCL);

    if (currentTime != TIME_INVALID)
    {
        Wire.beginTransmission(I2C_ADDR);

        app::secs_t delta = (app::secs_t)(millis() - currentTicks) / app::SECONDS;
        currentTime += RANGE_LIMIT(delta, 0, 120);

        Wire.write(I2C_ADDR);
        Wire.write('T');
        Wire.write((currentTime) & 0xFF);
        Wire.write((currentTime >> 8) & 0xFF);
        Wire.write((currentTime >> 16) & 0xFF);
        Wire.write((currentTime >> 24) & 0xFF);

        /* Checksum */
        Wire.write(BYTE_XOR('T', BYTE32(currentTime)));

        Wire.endTransmission();
    }

    while (true)
    {
        pinLed.blink(app::PinLed::NORM, 1);

        Wire.beginTransmission(I2C_ADDR);

        Wire.write(I2C_ADDR);
        Wire.write('S');
        Wire.write(LOBYTE(REPORT_INTERVAL));
        Wire.write(HIBYTE(REPORT_INTERVAL));

        /* Checksum */
        Wire.write(BYTE_XOR('S', BYTE16(REPORT_INTERVAL)));

        Wire.endTransmission();

        delay(3 * app::SECONDS);
    }
}
