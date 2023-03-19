#pragma once

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "PZEM004Tv30.h"

#include "TimerDispatcher.h"
#include "Reporter.h"
#include "PinBase.h"

namespace app {

// -------------------------------------------------------

class PinPzem : public TimerListener {
public:
    PinPzem(Reporter& reporter, float energyBase);
    ~PinPzem() = default;

    void init(float value);

    operator PZEM004Tv30() { return mPzem; }

    // Inplement TimerListener
    void onTimer();

    void sendMetric();
    
    float getValue();
    void setValue(float value);

private:
    Reporter& mReporter;

    SoftwareSerial mSerial;
    PZEM004Tv30 mPzem;

    float mEnergyBase;
    float mEnergySensor;
    
    secs mLastReportTime;
};

} // namespace fm
