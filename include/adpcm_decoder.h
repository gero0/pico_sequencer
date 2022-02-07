#ifndef __ADPCM_DEC__
#define __ADPCM_DEC__

#include<cstdint>

void adpcm_decoder_reset();
int16_t adpcm_decode_sample(int8_t nibble);

#endif