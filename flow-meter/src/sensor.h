#pragma once

#include <list>
#include <array>
#include <string>
#include <sstream>

#include <Arduino.h>
#include "OneWire.h"
#include "DallasTemperature.h"

#include <WiFiManager.h>

#include "TimerDispatcher.h"
#include "reporter.h"
#include "NonVolitileCounter.h"

#define PIN_D0      16
#define PIN_D1      5
#define PIN_D2      4
#define PIN_D3      0
#define PIN_D4      2
#define PIN_D5      14
#define PIN_D6      12
#define PIN_D7      13
#define PIN_D8      15
#define PIN_RX      3
#define PIN_TX      1

#define PINS_Dx     { PIN_D0, \
                      PIN_D1, \
                      PIN_D2, \
                      PIN_D3, \
                      PIN_D4, \
                      PIN_D5, \
                      PIN_D6, \
                      PIN_D7, \
                      PIN_D8, \
                      PIN_RX, \
                      PIN_RX }

#define SETUP_LED   2
#define SETUP_PIN   PIN_D1

namespace app {

enum Result {
    RESULT_OK = 0,
    RESULT_FAILED,
    RESULT_NOENT
};

class CustomText : public WiFiManagerParameter {
public:
    CustomText(const char * text) : WiFiManagerParameter(text) {}
    CustomText(const std::string & text) : WiFiManagerParameter(text.c_str()) {}
};

class BasePin {
public:
    BasePin(uint8_t pin): mPin(pin) {};
    ~BasePin() = default;
protected:
    uint8_t mPin;
};

class DSSensorPin : BasePin, public TimerListener {
public:
    class Sensor {
    public:
        Sensor() : address(), tempC(0.0) { address.fill(0); }
        ~Sensor() = default;

        operator uint8_t * () { return address.data(); };
        operator const std::string () const {
            std::stringstream ss;
            for (auto it = address.rbegin(); it != address.rend(); ++it) {
                ss << std::hex << std::uppercase << (int)*it;
            }
            return ss.str();
        }
        
        void addParameters(WiFiManager& wm);

    public:
        std::array<uint8_t, 8U> address;
        float tempC;
    private:
        uint16_t port;

        std::string mParamHeader;
    };
    using SensorIterator = std::list<Sensor>::iterator;

public:
    DSSensorPin(uint8_t pin, Reporter& reporter);
    ~DSSensorPin() = default;

    std::list<Sensor>::const_iterator begin() const;
    std::list<Sensor>::const_iterator end() const;

    void search();
    void read();

    app::Result get(std::string addr, float* out) const;

    void addParameters(WiFiManager& wm);

    void send(Reporter& reporter);

    // Implement TimerListener
    void onTimer() override;

private:
    OneWire mOneWire;
    Reporter& mReporter;

    std::list<Sensor> mSensors;
};

class InputPin : public BasePin, public TimerListener {
public:
    using VoidCallbackPtr = void (*)(void);
    
    InputPin(uint8_t pin, NonVolitileCounter& counter, Reporter& reporter, const char* name);
    ~InputPin() = default;

    void attach(VoidCallbackPtr callback) const;
    void process();

    uint32_t total() const;

    bool empty() const;
    void reset();

    operator uint32_t() const;
    InputPin& operator++();
    InputPin& operator--();

    void onInterrupt();

    void sendMetric();

    // Implement TimerListiner
    void onTimer();

private:
    std::string mName;
    Reporter& mReporter;

    uint64_t mBucket;
    NonVolitileCounter& mTotal;

    msec mLastState;
    msec mLastCheckMs;
};

class LedPin : public BasePin {
public:
    LedPin(uint8_t pin);
    LedPin(uint8_t pin, bool inverted);
    ~LedPin() = default;

    void setDimm(uint8_t dimm);

    void on() const;
    void off() const;
    void blink(unsigned long intervalMs = 100);
    void shortBlink();
private:
    uint8_t mDimm;
    uint64_t mLastMs;
    bool mInverted;
};

class IntParameter : public WiFiManagerParameter {
public:
    IntParameter(const char *id, const char *placeholder, long value, const uint8_t length = 10)
        : WiFiManagerParameter("") {
        init(id, placeholder, String(value).c_str(), length, "", WFM_LABEL_BEFORE);
    }

    long getValue() {
        return String(WiFiManagerParameter::getValue()).toInt();
    }
};

} // namespace fm