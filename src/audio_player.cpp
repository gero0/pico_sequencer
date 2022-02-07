#include "audio_player.h"
#include "adpcm_decoder.h"

static size_t position = 0;
static uint8_t* current_track = nullptr;
static size_t track_length = 0;
static bool is_playing = false;
static bool second_nibble = false;

void sound_reset() {
    is_playing = false;
    decoder_reset();
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
    decoder_reset();
    is_playing = true;
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

    if (!second_nibble) {
        uint8_t nibble = current_track[position] >> 4;
        int16_t sample = decode_sample(nibble);
        int sample_unsigned = 32768 + (int)sample; //values 0 - 65536
        int sample_10bit = ((float)sample_unsigned / 65536) * 1024;

        second_nibble = true;
        return (uint16_t)sample_10bit;
    }
    else {
        uint8_t nibble = current_track[position] & 15;
        int16_t sample = decode_sample(nibble);
        int sample_unsigned = 32768 + (int)sample; //values 0 - 65536
        int sample_10bit = ((float)sample_unsigned / 65536) * 1024;

        second_nibble = false;
        position++;
        return (uint16_t)sample_10bit;
    }

}
