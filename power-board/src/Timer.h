#pragma once

#include <Arduino.h>

namespace app {

using msec = unsigned long;
using secs = unsigned long;

static const msec SECONDS = 1000;
static const msec MINUTES = 60000;

static const msec TIME_INVALID = (-1);

} // namespace app