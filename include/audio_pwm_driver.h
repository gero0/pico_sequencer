#ifndef __AUDIO_DRIVER
#define __AUDIO_DRIVER

#include <pico/stdlib.h>

void audio_init(int audio_pin);
bool audio_timer_callback(struct repeating_timer* t);

#endif