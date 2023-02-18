#pragma once

#include <list>

#include <Arduino.h>

namespace app {

using msec = unsigned long;
using secs = unsigned long;

static const msec SECONDS = 1000;
static const msec MINUTES = 60000;

static const msec TIME_INVALID = (-1);

// --------------------------------------------------------

class TimerListener {
public:
    TimerListener() {};
    ~TimerListener() = default;

    virtual void onTimer() = 0;
};

// --------------------------------------------------------

class TimerDispatcher {
public:
    TimerDispatcher();
    ~TimerDispatcher() = default;

    void startTimer(TimerListener& listener, msec interval, bool runOnce = false);

    void process();

private:

    struct Timer {
        TimerListener& listener;
        msec nextTime;
        msec interval;
    };

    std::list<Timer> mTimers;
};

} // namespace app