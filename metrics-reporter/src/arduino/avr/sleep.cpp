#include <avr/sleep.h>

uint8_t ADCSRA = 0;
uint8_t ADEN = 0;
uint8_t WDTCR = 0;

uint8_t g_sleep_mode = SLEEP_MODE_UNDEFUNED;
bool g_sleep_enabled = false;
bool g_sleep_cpu = false;
