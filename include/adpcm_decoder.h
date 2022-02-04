#ifndef __ADPCM_DEC__
#define __ADPCM_DEC__

#include<cstdint>

void decoder_reset();
int16_t decode_sample(int8_t nibble);

#endif