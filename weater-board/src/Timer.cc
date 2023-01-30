#include <Arduino.h>

#include "Timer.h"

namespace app {

// --------------------------------------------------------

TimerDispatcher::TimerDispatcher() {}

void TimerDispatcher::startTimer(TimerListener& listener, msec interval, bool runOnce) {

    Timer timer = {
        .listener = listener,
        .nextTime = (millis() + interval),
        .interval = (runOnce) ? TIME_INVALID : interval
    };
    mTimers.push_back(timer);
}

void TimerDispatcher::process() {

    msec currentTimeMs = millis();

    for (auto it = mTimers.begin(); it != mTimers.end();) {

        if (currentTimeMs >= (*it).nextTime) {

            (*it).listener.onTimer();

            if ((*it).interval == TIME_INVALID) {
                it = mTimers.erase(it);
            }
            else {
                (*it).nextTime += (*it).interval;
            }
        }
        else {
            ++it;
        }
    }
}

} // namespace app