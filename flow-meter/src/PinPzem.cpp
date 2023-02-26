#include <Arduino.h>
#include <iomanip>

#include "PinPzem.h"

namespace app {

// -------------------------------------------------------

PinPzem::PinPzem(Reporter& reporter, float *energyCorrection_p)
    : mReporter(reporter),
      mSerial(UART0),
      mPzem(mSerial),
      mEnergy(0.0),
      mEnergyReset(false),
      mEnergyCorrection_p(energyCorrection_p),
      mLastReportTime(app::TIME_INVALID) {
    
    mSerial.begin(9600);
}

void PinPzem::onTimer() {
    sendMetric();
}

void PinPzem::sendMetric() {

    secs currentTime = millis() / app::SECONDS;

    if (mEnergyReset) {
        mPzem.resetEnergy();
    }

    float voltage = mPzem.voltage();
    float current = mPzem.current();
    float power = mPzem.power();
    float energy = mPzem.energy();
    float frequency = mPzem.frequency();

    if (!std::isnan(voltage) &&
        !std::isnan(current) &&
        !std::isnan(power) &&
        !std::isnan(energy) &&
        !std::isnan(frequency)) {

        std::stringstream ss;

        ss << std::fixed
           << "test,sensor=pzem"
           << " voltage="   << std::setprecision(1) << voltage
           << ",current="   << std::setprecision(3) << current
           << ",frequency=" << std::setprecision(1) << frequency
           << ",power="     << std::setprecision(2) << power
           << ",energy="    << std::setprecision(3) << (*mEnergyCorrection_p + energy);
 
        if ((mEnergy != 0.0) && (mLastReportTime != app::TIME_INVALID)) {
        
            ss << ",delta="    << std::setprecision(3) << (energy - mEnergy)
               << ",duration=" << std::setprecision(3) << (currentTime - mLastReportTime);
        }
 
        mReporter.send(ss.str());
    }

    mEnergy = energy;
    mLastReportTime = currentTime;
}

void PinPzem::resetMetric() {
    mEnergyReset = true;
}

} // namespace app 