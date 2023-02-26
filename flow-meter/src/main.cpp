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
#include "PinCounter.h"
#include "WmConfig.h"

app::PinLed setupLed(SETUP_LED, true);
app::PinLed redLed(PIN_D3);

// Settings
struct Settings {
    float pzemEnergy;
    long coldWaterCounter;
    long hotWaterCounter;
};

SoftwareSerial debug(PIN_RX2, PIN_TX2);

WiFiManager wm;
Settings settings = {0};

app::TimerDispatcher td;

app::Reporter reporter("192.168.0.5", 42001);

app::NonVolitileCounter hotWaterCounter(128, 30);
app::NonVolitileCounter coldWaterCounter(256, 30);

app::PinCounter hotCounter(PIN_D6, hotWaterCounter, reporter, "hot", 10);
app::PinCounter coldCounter(PIN_D5, coldWaterCounter, reporter, "cold", 10);

app::DSSensorPin sensors(PIN_D2, reporter);

app::PinPzem pzem(reporter, settings.pzemEnergy);

app::FloatParameter paramPzemEnergy("pzem_energy", "PZEM Energy (kWh)", 0.0);
app::IntParameter paramColdWaterCounter("cold_counter", "Cold Water Counter (L)", 0);
app::IntParameter paramHotWaterCounter("hot_counter", "Hot Water Counter (L)", 0);

void saveParamsCallback () {
  
    Settings oldSettings = settings;

    settings.pzemEnergy = paramPzemEnergy.getValue();
    settings.coldWaterCounter = paramColdWaterCounter.getValue();
    settings.hotWaterCounter = paramHotWaterCounter.getValue();

    EEPROM.put(0, settings);
    if (EEPROM.commit()) {
        debug.println("Settings saved");
    } else {
        debug.println("EEPROM error");
    }

    if (oldSettings.pzemEnergy != settings.pzemEnergy) {
        pzem.setValue(settings.pzemEnergy);
    }

    if (oldSettings.coldWaterCounter != settings.coldWaterCounter) {
        coldCounter.setValue(settings.coldWaterCounter);
    }

    if (oldSettings.hotWaterCounter != settings.hotWaterCounter) {
        hotCounter.setValue(settings.hotWaterCounter);
    }
}

void setup() {

    debug.begin(115200);
    delay(1000);

    WiFi.mode(WIFI_STA);

    redLed.setDimm(30);
    redLed.blink();

    sensors.search();

    pinMode(SETUP_PIN, INPUT_PULLUP);
    delay(1000);

    // Read EEPROM settings
    EEPROM.begin(512);
    EEPROM.get(0, settings);
    debug.println("Settings loaded");

    // Initialize non-volitile counters
    coldWaterCounter.init(settings.coldWaterCounter);
    hotWaterCounter.init(settings.hotWaterCounter);

    // WifiManager parameters
    paramPzemEnergy.setValue(String(settings.pzemEnergy).c_str(),
                             paramPzemEnergy.getValueLength());

    paramColdWaterCounter.setValue(String(settings.coldWaterCounter).c_str(),
                                   paramColdWaterCounter.getValueLength());

    paramHotWaterCounter.setValue(String(settings.hotWaterCounter).c_str(),
                                  paramHotWaterCounter.getValueLength());

    wm.addParameter(&paramPzemEnergy);
    wm.addParameter(&paramColdWaterCounter);
    wm.addParameter(&paramHotWaterCounter);

    if (digitalRead(SETUP_PIN) == LOW) {

        debug.println("Setup pin is ON");
        debug.println("-- SETUP --");
       
        std::vector<const char*> menu = {"wifi", "param", "info", "exit", "sep", "update"};
        wm.setMenu(menu);

        wm.startConfigPortal();

        settings.pzemEnergy = paramPzemEnergy.getValue();
        settings.coldWaterCounter = paramColdWaterCounter.getValue();
        settings.hotWaterCounter = paramHotWaterCounter.getValue();

        EEPROM.put(0, settings);
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