#include "audio_player.h"
#include "adpcm_decoder.h"

static size_t position = 0;
static uint8_t* current_track = nullptr;
static size_t track_length = 0;
static bool is_playing = false;
static bool second_nibble = false;

void sound_reset() {
    is_playing = false;
    adpcm_decoder_reset();
    current_track = nullptr;
    track_length = 0;
    position = 0;
    second_nibble = false;
}

void set_track(uint8_t* track, size_t track_len) {
    current_track = track;
    track_length = track_len;
    position = 0;
    second_nibble = false;
    adpcm_decoder_reset();
    is_playing = true;
}

int convert_sample(int16_t sample){
    int sample_unsigned = 32768 + (int)sample; //values 0 - 65536
    int sample_10bit = ((float)sample_unsigned / 65536) * 1024;

    return sample_10bit;
}

uint8_t get_next_nibble(){
    uint8_t nibble = 0;

    if (!second_nibble) {
        nibble = current_track[position] >> 4;
    }
    else {
        nibble = current_track[position] & 15;
        position++;
    }

    second_nibble = !second_nibble;
    return nibble;
}

uint16_t get_next_sample() {
    if (!is_playing) {
        return 0;
    }

    if (position >= track_length) {
        //playback ended
        sound_reset();
        return 0;
    }

    uint8_t nibble = get_next_nibble();
    int16_t sample = adpcm_decode_sample(nibble);
    int sample_10bit = convert_sample(sample);
    
    return uint16_t(sample_10bit);
}

