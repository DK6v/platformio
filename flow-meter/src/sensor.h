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
#include "Reporter.h"
#include "NonVolitileCounter.h"

#include "PinBase.h"

namespace app {

class CustomText : public WiFiManagerParameter {
public:
    CustomText(const char * text) : WiFiManagerParameter(text) {}
    CustomText(const std::string & text) : WiFiManagerParameter(text.c_str()) {}
};

class DSSensorPin : PinBase, public TimerListener {
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

class InputPin : public PinBase, public TimerListener {
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