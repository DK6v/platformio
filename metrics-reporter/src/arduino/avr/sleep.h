#pragma once

#include <stdint.h>

extern uint8_t ADCSRA;
extern uint8_t ADEN;
extern uint8_t WDTCR;

extern uint8_t g_sleep_mode;
extern bool g_sleep_enabled;
extern bool g_sleep_cpu;

#define WDIF    7
#define WDIE    6
#define WDP3    5
#define WDCE    4
#define WDE     3
#define WDP2    2
#define WDP1    1
#define WDP0    0

#define SLEEP_MODE_UNDEFUNED    (-1)
#define SLEEP_MODE_IDLE         0
#define SLEEP_MODE_ADC          1
#define SLEEP_MODE_PWR_DOWN     2
#define SLEEP_MODE_PWR_SAVE     3

#define set_sleep_mode(mode) do { g_sleep_mode = mode; } while (0)
#define sleep_enable()       do { g_sleep_enabled = true; } while (0)
#define sleep_disable()      do { g_sleep_enabled = true; } while (0)
#define sleep_cpu()          do { g_sleep_cpu = true; } while (0)
