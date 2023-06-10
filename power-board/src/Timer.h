#pragma once

#include <Arduino.h>

namespace app {

using msec_t = signed long;
using secs_t = signed long;

static const msec_t SECOND = 1000;
static const msec_t MINUTE = 60000;

static const msec_t TIME_INVALID = ~(1L << ((sizeof(msec_t) * 8) - 1));

} // namespace app