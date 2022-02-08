#include <hardware/pwm.h>
#include "audio_player.h"
#include "audio_pwm_driver.h"
#include "sound_mappings.h"

static repeating_timer_t audio_timer;

static int audio_pwm_pin = 0;
static int audio_pin_slice = 0;

void audio_init(int audio_pin) {
    audio_pwm_pin = audio_pin;
    gpio_set_function(audio_pwm_pin, GPIO_FUNC_PWM);
    audio_pin_slice = pwm_gpio_to_slice_num(audio_pwm_pin);
    pwm_config config = pwm_get_default_config();

    pwm_config_set_clkdiv(&config, 1.0f);
    pwm_config_set_wrap(&config, 1000); // 125kHz
    pwm_init(audio_pin_slice, &config, true);

    pwm_set_gpio_level(audio_pwm_pin, 0);
    add_repeating_timer_us(23, audio_timer_callback, NULL, &audio_timer); //23us for approx. 44100kHz
}

void audio_play_sound(size_t note) {
    const char* sound = instrument_sounds[note];
    const size_t sound_len = instrument_sounds_len[note];
    if (sound != nullptr) {
        audioplayer_set_track((uint8_t*)sound, sound_len);
    }
}

bool audio_timer_callback(struct repeating_timer* t) {
    uint16_t sample = audioplayer_get_next_sample();
    pwm_set_gpio_level(audio_pwm_pin, sample);
    return true;
}