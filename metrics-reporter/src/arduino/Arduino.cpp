#include <Arduino.h>

uint64_t g_millis = 0;
uint64_t g_delay = 0;

bool g_is_interupts_enabled = true;

uint64_t millis() {
    return g_millis;
}

void delay(unsigned long ms) {
    g_delay = ms;
}