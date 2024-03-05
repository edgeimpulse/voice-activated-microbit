// WS2812B timings, +-0.15uS
// 0 - 0.40uS hi 0.85uS low
// 1 - 0.80uS hi 0.45uS low

#ifndef CODAL_NEOPIXEL_H
#define CODAL_NEOPIXEL_H

#ifndef HARDWARE_NEOPIXEL
#define HARDWARE_NEOPIXEL     0
#endif

#include "NRF52PWM.h"
#include "WS2812B.h"

void neopixel_send_buffer(Pin &pin, const uint8_t *ptr, int numBytes);
void neopixel_send_buffer(Pin &pin, ManagedBuffer buffer);

#endif // CODAL_NEOPIXEL_H
