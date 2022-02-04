//based on http://www.cs.columbia.edu/~hgs/audio/dvi/IMA_ADPCM.pdf

#include<cstdint>

/* Table of index changes */
const int16_t indexTable[16] = { -1,-1,-1,-1, 2, 4, 6, 8, -1,-1,-1,-1, 2, 4, 6, 8 };

/* quantizer lookup table */
const int16_t stepsizeTable[89] = {
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28,
    31, 34, 37, 41, 45, 50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
    130, 143, 157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408, 449,
    494, 544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552,
    1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871,
    5358, 5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

static int predicted_sample = 0;
static int16_t index = 0;
static int16_t step_size = stepsizeTable[index];

void decoder_reset() {
    predicted_sample = 0;
    index = 0;
    step_size = stepsizeTable[index];
}

int16_t decode_sample(int8_t nibble) {
    
    int16_t diff = 0;

    //some black magic fast multiplication stuff
    if(nibble & 4)
        diff += step_size;
    if(nibble & 2)
        diff += step_size >> 1;
    if(nibble & 1)
        diff += step_size >> 2;

    diff += step_size >> 3;

    if(nibble & 8)
        diff = -diff;

    predicted_sample += diff;

    if (predicted_sample > 32767) {
        predicted_sample = 32767;
    }
    else if (predicted_sample < -32768) {
        predicted_sample = -32768;
    }

    index = index + indexTable[nibble];

    if (index < 0) /* check for index underflow */
        index = 0;
    else if (index > 88) /* check for index overflow */
        index = 88;

    step_size = stepsizeTable[index];

    return (uint16_t)predicted_sample;
}