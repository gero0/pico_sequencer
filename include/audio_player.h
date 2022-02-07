#ifndef __AUDIO_PLAYER
#define __AUDIO_PLAYER

#include <cstdint>
#include <cstdlib>

void audioplayer_sound_reset();
void audioplayer_set_track(uint8_t* track, size_t track_len);
uint16_t audioplayer_get_next_sample();


#endif