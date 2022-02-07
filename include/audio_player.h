#ifndef __AUDIO_PLAYER
#define __AUDIO_PLAYER

#include <cstdint>
#include <cstdlib>

void sound_reset();
void set_track(uint8_t* track, size_t track_len);
uint16_t get_next_sample();


#endif