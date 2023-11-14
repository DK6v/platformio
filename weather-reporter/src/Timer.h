#pragma once

#include <list>

#include <Arduino.h>

namespace app {

using secs_t = signed long long;
using msec_t = signed long long;
using usec_t = signed long long;

using ticks_t = unsigned long long;

enum : signed long long{
    TIME_INVALID = ~(1LL << (8 * sizeof(signed long long) - 1)),
    TICKS_INVALID = ~(0)
};

static const msec_t SECONDS = 1000;
static const msec_t MINUTE = 60000;

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

    void startTimer(TimerListener& listener, msec_t interval, bool runOnce = false);

    void process();

private:

    struct Timer {
        TimerListener& listener;
        unsigned long nextTime;
        msec_t interval;
    };

    std::list<Timer> mTimers;
};

} // namespace app