#include <Arduino.h>
#include <WiFiManager.h>
#include <EEPROM.h>

#if defined(ESP8266)
#include <ESP8266WebServer.h>
#endif

#include "TimerDispatcher.h"
#include "sensor.h"
#include "Reporter.h"
#include "NonVolitileCounter.h"
#include "PinLed.h"
#include "PinPzem.h"

namespace app {

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

app::PinLed setupLed(SETUP_LED, true);
app::PinLed redLed(PIN_D3);

// Settings
struct Settings {
    float pzemEnergy;
} sett;

WiFiManager wm;

int intParam = 0;

app::TimerDispatcher td;

app::Reporter reporter("192.168.0.5", 42001);

app::NonVolitileCounter hotWaterCounter(128, 30);
app::NonVolitileCounter coldWaterCounter(256, 30);

app::InputPin hotCounter(PIN_D6, hotWaterCounter, reporter, "hot");
app::InputPin coldCounter(PIN_D5, coldWaterCounter, reporter, "cold");

app::DSSensorPin sensors(PIN_D2, reporter);

app::PinPzem pzem(reporter, &sett.pzemEnergy);

SoftwareSerial debug(PIN_RX2, PIN_TX2);

app::FloatParameter pzemEnergy( "float", "PZEM Energy (kWh)", sett.pzemEnergy);
WiFiManagerParameter paramSeparator("<hr><br/>");

void saveParamsCallback () {
  
    sett.pzemEnergy = pzemEnergy.getValue();

    EEPROM.put(0, sett);
    if (EEPROM.commit()) {
        debug.println("Settings saved");
    } else {
        debug.println("EEPROM error");
    }

    pzem.resetMetric();
}

void setup() {

    debug.begin(115200);
    delay(1000);

    WiFi.mode(WIFI_STA);

    redLed.setDimm(30);
    redLed.blink();

    sensors.search();

    EEPROM.begin(512);
    hotWaterCounter.init(10101);
    coldWaterCounter.init(13692);

    EEPROM.get(0, sett);
    pzemEnergy.setValue(String(sett.pzemEnergy).c_str(), pzemEnergy.getValueLength());

    debug.println("Settings loaded");

    pinMode(SETUP_PIN, INPUT_PULLUP);
    delay(1000);

    wm.addParameter(&pzemEnergy);

    if (digitalRead(SETUP_PIN) == LOW) {

        debug.println("Setup pin is ON");
        debug.println("-- SETUP --");
       
        hotWaterCounter = 10101;
        coldWaterCounter = 13692;

        std::vector<const char *> menu = {"wifi","param","info","exit","sep","update"};
        wm.setMenu(menu); // custom menu, pass vector

        wm.startConfigPortal();

        sett.pzemEnergy = pzemEnergy.getValue();

        EEPROM.put(0, sett);
        if (EEPROM.commit()) {
            debug.println("Settings saved");
        } else {
            debug.println("EEPROM error");
        }
    } 
    else {
        debug.println("Setup pin is OFF");
        debug.println("-- WORK --");
      
        wm.setSaveParamsCallback(saveParamsCallback);

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
    td.startTimer(pzem,             (5  * app::MINUTES));

    debug.print("Water counters, cold=");
    debug.print(coldWaterCounter);
    debug.print(", hot=");
    debug.println(hotWaterCounter);

    redLed.blink();
    redLed.blink();
}

#define TICK_MS (10)

void loop() {  

    coldCounter.process();
    hotCounter.process();

    wm.process();
    td.process();

    delay(TICK_MS);
}