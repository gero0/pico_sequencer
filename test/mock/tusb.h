#ifndef __TUSB__
#define __TUSB__

#include <cstdint>

uint32_t tud_midi_stream_write(uint8_t, uint8_t*, uint32_t);
int tud_midi_available();
int tud_midi_packet_read(unsigned char *);

#endif