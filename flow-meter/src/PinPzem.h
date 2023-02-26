#pragma once

#include <Arduino.h>
#include <HardwareSerial.h>
#include "PZEM004Tv30.h"

#include "TimerDispatcher.h"
#include "Reporter.h"
#include "PinBase.h"

namespace app {

// -------------------------------------------------------

class PinPzem : public TimerListener {
public:
    PinPzem(Reporter& reporter, float *energyCorrection_p);
    ~PinPzem() = default;

    operator PZEM004Tv30() { return mPzem; }

    // Inplement TimerListener
    void onTimer();

    void sendMetric();
    void resetMetric();

private:
    Reporter& mReporter;

    HardwareSerial mSerial;
    PZEM004Tv30 mPzem;

    float mEnergy;
    bool mEnergyReset;
    float *mEnergyCorrection_p;  

    secs mLastReportTime;
};

} // namespace fm
