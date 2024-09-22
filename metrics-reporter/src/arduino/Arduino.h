#pragma once

#include <stdint.h>

#define abs(x) ((x)>0?(x):-(x))
#define bit(b) (1UL << (b))

#define cli()  do { g_is_interupts_enabled = false; } while (0)
#define sei()  do { g_is_interupts_enabled = true; } while (0)

extern uint64_t g_millis;
extern uint64_t g_delay;
extern bool     g_is_interupts_enabled;

uint64_t millis();
void delay(unsigned long ms);
