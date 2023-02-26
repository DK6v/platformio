#include "sensor.h"

namespace app {

// --------------------------------------------------------

DSSensorPin::DSSensorPin(uint8_t pin, Reporter& reporter):
    PinBase(pin),
    mOneWire(pin),
    mReporter(reporter),
    mSensors() {

    pinMode(pin, INPUT);
}

std::list<DSSensorPin::Sensor>::const_iterator DSSensorPin::begin() const {
    return mSensors.begin();
}
std::list<DSSensorPin::Sensor>::const_iterator DSSensorPin::end() const {
    return mSensors.end();
}

void DSSensorPin::search() {
    mOneWire.reset_search();
    mSensors.clear();

    Sensor sensor;
    while(true == mOneWire.search(sensor)) {
        mSensors.push_back(sensor);

        Serial.print("Sensor search: ");
        Serial.println(static_cast<std::string>(sensor).c_str());
    }
}

void DSSensorPin::read() {

    DallasTemperature bus(&mOneWire);
    
    bus.begin();
    bus.requestTemperatures();

    for (auto & sensor : mSensors) {

        sensor.tempC = bus.getTempC(sensor);

        Serial.print("Sensor read: ");
        Serial.print(static_cast<std::string>(sensor).c_str());
        Serial.print(", temperature(C): ");
        Serial.println(sensor.tempC);
    }
}

app::Result DSSensorPin::get(std::string addr, float* out) const {
    for (auto & sensor : mSensors) {
        if (addr.compare(addr) == 0) {
            *out = sensor.tempC;
            return app::RESULT_OK;
        }
    }
    return app::RESULT_NOENT;
} 

void DSSensorPin::addParameters(WiFiManager& wm) {
    for (auto & sensor : mSensors) {
        sensor.addParameters(wm);
    }
}

void DSSensorPin::Sensor::addParameters(WiFiManager& wm) {   
    
    mParamHeader = std::string("Sensor ") +
                   static_cast<std::string>(*this) +
                   std::string("<hr><br/>");

    CustomText paramHeader(mParamHeader.c_str());
    wm.addParameter( &paramHeader );
}

void DSSensorPin::send(Reporter& reporter) {
    for (auto & sensor: mSensors) {
        std::string metric = "temp,sensor=0x" + std::string(sensor)
                           + " temp=" + std::to_string(sensor.tempC);
        reporter.send(metric);
    }
}

void DSSensorPin::onTimer() {
    read();
    send(mReporter);
}

// ------------------------------------------------------------

InputPin::InputPin(uint8_t pin,
                   NonVolitileCounter& counter,
                   Reporter& reporter,
                   const char* name):
    PinBase(pin),
    mName(name),
    mReporter(reporter),
    mBucket(0),
    mTotal(counter) {
    
    pinMode(pin, INPUT);

    mLastState = digitalRead(mPin);
    mLastCheckMs = millis();
}

void InputPin::attach(VoidCallbackPtr callback) const {
    pinMode(mPin, INPUT);
    attachInterrupt(mPin, callback, ONLOW);
}

void InputPin::process() {
    
    pinMode(mPin, INPUT);

    msec currentTimeMs = millis();

    if (mLastCheckMs < (currentTimeMs - 1000)) {

        if ((mLastState == LOW) && (digitalRead(mPin) == HIGH)) {

            mLastState = HIGH;

            ++mBucket;
            ++mTotal;
        }
        else if ((mLastState == HIGH) && (digitalRead(mPin) == LOW)) {

            mLastState = LOW;
        }

        mLastCheckMs = currentTimeMs;
    }
}

uint32_t InputPin::total() const {
    return mTotal;
}

bool InputPin::InputPin::empty() const {
    return (mBucket == 0);
}

void InputPin::reset() {
    mBucket = 0;
}

InputPin::operator uint32_t() const {
    return mBucket;
}

InputPin& InputPin::operator++() {
    ++mBucket;
    return *this;
}

InputPin& InputPin::operator--() {
    if (mBucket != 0) {
        --mBucket;
    };
    return *this;
}

void InputPin::onInterrupt() {
    
    msec currentTimeMs = millis();
    
    if (mLastCheckMs < (currentTimeMs - 20)) {
        ++mBucket;
        ++mTotal;
    }
    mLastCheckMs = currentTimeMs;
}

void InputPin::sendMetric() {

    if (mBucket != 0) {

        std::string metric = "water,sensor=" + std::string(mName)
                           + " count=" + std::to_string(mBucket * 10)
                           + ",total=" + std::to_string(mTotal * 10);

        if (0 != mReporter.send(metric)) {
            mBucket = 0;
        }
    }
}

void InputPin::onTimer() {

    sendMetric();
}

} // namespace fm