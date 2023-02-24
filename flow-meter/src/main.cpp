#include <Arduino.h>
#include <WiFiManager.h>
#include <EEPROM.h>

#if defined(ESP8266)
#include <ESP8266WebServer.h>
#endif

#include "TimerDispatcher.h"
#include "sensor.h"
#include "reporter.h"
#include "NonVolitileCounter.h"

namespace app {

class IPAddressParameter : public WiFiManagerParameter {
public:
    IPAddressParameter(const char *id, const char *placeholder, IPAddress address)
        : WiFiManagerParameter("") {
        init(id, placeholder, address.toString().c_str(), 16, "", WFM_LABEL_BEFORE);
    }

    bool getValue(IPAddress &ip) {
        return ip.fromString(WiFiManagerParameter::getValue());
    }
};

class FloatParameter : public WiFiManagerParameter {
public:
    FloatParameter(const char *id, const char *placeholder, float value, const uint8_t length = 10)
        : WiFiManagerParameter("") {
        init(id, placeholder, String(value).c_str(), length, "", WFM_LABEL_BEFORE);
    }

    float getValue() {
        return String(WiFiManagerParameter::getValue()).toFloat();
    }
};

} // namespace fm

app::LedPin setupLed(SETUP_LED, true);
app::LedPin redLed(PIN_D3);

// Settings
struct Settings {
    float f;
    int i;
    char s[20];
    uint32_t ip;
} sett;

WiFiManager wm;

int intParam = 0;

app::IntParameter param_int( "int", "param_int",  intParam);
WiFiManagerParameter param_sep("<hr><br/>"); // separator

app::TimerDispatcher td;

app::Reporter reporter("192.168.0.5", 42001);

app::NonVolitileCounter hotWaterCounter(128, 30);
app::NonVolitileCounter coldWaterCounter(256, 30);

app::InputPin hotCounter(PIN_D6, hotWaterCounter, reporter, "hot");
app::InputPin coldCounter(PIN_D5, coldWaterCounter, reporter, "cold");

app::DSSensorPin sensors(PIN_D2, reporter);

void setup() {

    Serial.begin(115200);
    delay(1000);

    WiFi.mode(WIFI_STA);

    redLed.setDimm(30);
    redLed.blink();

    sensors.search();

    EEPROM.begin(512);
    hotWaterCounter.init(10101);
    coldWaterCounter.init(13692);

    EEPROM.get(0, sett);
    Serial.println("Settings loaded");

    pinMode(SETUP_PIN, INPUT_PULLUP);
    delay(1000);

    if (digitalRead(SETUP_PIN) == LOW) {

        Serial.println("Setup pin is ON");
        Serial.println("-- SETUP --");
       
        hotWaterCounter = 10101;
        coldWaterCounter = 13692;

        redLed.blink();

        sett.s[19] = '\0'; //add null terminator at the end cause overflow
        WiFiManagerParameter param_str( "str", "param_string",  sett.s, 20);
        app::FloatParameter param_float( "float", "param_float",  sett.f);
        app::IntParameter param_int( "int", "param_int",  sett.i);
        WiFiManagerParameter param_sep("<hr><br/>"); // separator

        IPAddress ip(sett.ip);
        app::IPAddressParameter param_ip("ip", "param_ip", ip);

        sensors.addParameters(wm);
        
        wm.addParameter( &param_str );
        wm.addParameter( &param_float );
        wm.addParameter( &param_sep );
        wm.addParameter( &param_int );
        wm.addParameter( &param_sep );
        wm.addParameter( &param_ip );

        std::vector<const char *> menu = {"wifi","param","info","exit","sep","update"};
        wm.setMenu(menu); // custom menu, pass vector

        //SSID & password parameters already included
        wm.startConfigPortal();

        strncpy(sett.s, param_str.getValue(), 20);
        sett.s[19] = '\0'; 
        sett.f = param_float.getValue();
        sett.i = param_int.getValue();

        Serial.print("String param: ");
        Serial.println(sett.s);
        Serial.print("Float param: ");
        Serial.println(sett.f);
        Serial.print("Int param: ");
        Serial.println(sett.i, DEC);
        
        if (param_ip.getValue(ip)) {
            sett.ip = ip;

            Serial.print("IP param: ");
            Serial.println(ip);
        } else {
            Serial.println("Incorrect IP");
        }

        EEPROM.put(0, sett);
        if (EEPROM.commit()) {
            Serial.println("Settings saved");
        } else {
            Serial.println("EEPROM error");
        }
    } 
    else {
        Serial.println("Setup pin is OFF");
        Serial.println("-- WORK --");
      
        wm.addParameter( &param_int );
        wm.addParameter( &param_sep );

        std::vector<const char *> menu = {"wifi", "param", "info", "exit"};

        wm.setMenu(menu); // custom menu, pass vector

        wm.autoConnect();
        wm.startWebPortal();
    }

    delay(1000);

    td.startTimer(hotCounter,       (60 * app::SECONDS));
    td.startTimer(coldCounter,      (60 * app::SECONDS));
    td.startTimer(sensors,          (5  * app::MINUTES));
    td.startTimer(hotWaterCounter,  (15 * app::MINUTES));
    td.startTimer(coldWaterCounter, (15 * app::MINUTES));

    Serial.print("Water counters, cold=");
    Serial.print(coldWaterCounter);
    Serial.print(", hot=");
    Serial.println(hotWaterCounter);
}

#define TICK_MS (10)

void loop() {  

    coldCounter.process();
    hotCounter.process();

    wm.process();
    td.process();

    delay(TICK_MS);
}