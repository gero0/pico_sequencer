#include <stdio.h>
#include <algorithm>
#include "pico/stdlib.h"

#include "bsp/board.h"
#include "tusb.h"
#include "usb_descriptors.h"

#include "pins.h"
#include "display.h"
#include "constant_arr.h"

int recv_midi();
int bpm_to_delay(int bpm);
void seq_leds();
void display();
void set_interrupts();
void scan_inputs(bool* button_states);
void pattern_button_pressed(uint8_t button);
void update_display();

void stop();
void start();

bool note_timer_callback(struct repeating_timer* t);
void button_irq(uint gpio, uint32_t events);

void test_button_handler();
void start_stop_button_handler();
void set_button_handler();
void encoder_handler();

static repeating_timer_t note_timer;
static repeating_timer_t display_timer;

static uint8_t midi_channel = 10;
static const uint8_t cable_num = 0; // MIDI jack associated with USB endpoint

//0 - no note played (why would we want to play 8.18Hz anyway? ;)
//uint8_t sequence[] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
static uint8_t sequence[] = { 36,0,42,0,36,0,42,0,36,0,42,0,36,0,42,0 };
static uint32_t seq_pos = 0;
static uint8_t last_note = 0;

static uint32_t global_tempo = 120;

static uint8_t velocity = 127;

enum PlayingState {
    STOPPED,
    PLAYING,
};

static enum PlayingState playing_state = STOPPED;

static absolute_time_t last_startstop_int_time;
static absolute_time_t last_test_int_time;
static absolute_time_t last_set_int_time;
static absolute_time_t last_enc_int_time;
static absolute_time_t last_screen_update_time;

static uint8_t selected_note = 39;

//Expansion ideas: 
//MIDI Port
//Saving patterns to eeprom

int main() {
    sleep_ms(1000);
    board_init();
    tusb_init();
    configure_pins();
    setup_default_uart();
    set_interrupts();
    seq_leds();

    LCD_init();

    //Start/stop button irq
    last_startstop_int_time = get_absolute_time();
    last_test_int_time = get_absolute_time();
    last_set_int_time = get_absolute_time();
    last_enc_int_time = get_absolute_time();

    last_screen_update_time = get_absolute_time();

    add_repeating_timer_ms(bpm_to_delay(global_tempo), note_timer_callback, NULL, &note_timer);

    while (true) {
        tud_task(); // tinyusb device task
        recv_midi();
        seq_leds();

        if (absolute_time_diff_us(last_screen_update_time, get_absolute_time()) > 200000) {
            update_display();
            last_screen_update_time = get_absolute_time();
        }

        //scan twice to debounce
        bool button_states[] = { false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false };
        scan_inputs(button_states);

        // sleep_ms(10);

        bool button_states2[] = { false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false };
        scan_inputs(button_states2);

        //detect input only if detected in both scans
        for (int i = 0; i < 16; i++) {
            if (button_states[i] == button_states2[i]) {
                if (button_states[i]) {
                    pattern_button_pressed(i);
                }
            }
        }
    }
}

void set_interrupts() {
    gpio_set_irq_enabled_with_callback(
        START_STOP_BTN,
        GPIO_IRQ_EDGE_RISE,
        true,
        button_irq
    );

    gpio_set_irq_enabled_with_callback(
        TEST_BTN,
        GPIO_IRQ_EDGE_RISE,
        true,
        button_irq
    );

    gpio_set_irq_enabled_with_callback(
        SET_BTN,
        GPIO_IRQ_EDGE_RISE,
        true,
        button_irq
    );

    gpio_set_irq_enabled_with_callback(
        ENC_1,
        GPIO_IRQ_EDGE_FALL,
        true,
        button_irq
    );
}

int recv_midi() {
    uint8_t packet[4];
    if (tud_midi_available()) {
        bool result = tud_midi_packet_read(packet);

        if (result) {
            // for (int i = 0; i < 4; i++)
            //     printf("%X ", packet[i]);
            // printf("\n");
        }

        return 4;
    }
    else {
        return 0;
    }
}

void seq_leds() {
    for (int i = 0; i < 16; i++) {
        //light up steps with this note and also light up current step LED during playing
        bool led_state = (sequence[i] == selected_note || (playing_state == PLAYING && i == seq_pos));
        gpio_put(SHIFT_DATA, led_state);
        gpio_put(CLOCK, 1);
        //sleep?
        gpio_put(CLOCK, 0);
    }

    gpio_put(LATCH, 1);
    //sleep?
    gpio_put(LATCH, 0);
}

int bpm_to_delay(int bpm) {
    float seconds_per_beat = 60.0 / bpm;
    float seconds_per_sixteenth = seconds_per_beat / 4;
    float ms_per_sixteenth = seconds_per_sixteenth * 1000;
    return (int)ms_per_sixteenth;
}

bool note_timer_callback(struct repeating_timer* t) {
    if (playing_state == PLAYING) {
        if (last_note != 0) {
            uint8_t note_off[3] = { 0x80 | midi_channel, last_note, 0 };
            tud_midi_stream_write(cable_num, note_off, 3);
        }

        uint8_t note = sequence[seq_pos];
        if (note != 0) {
            uint8_t note_on[3] = { 0x90 | midi_channel, note, velocity };
            tud_midi_stream_write(cable_num, note_on, 3);
            last_note = note;
        }
    }

    if (!gpio_get(HOLD_BTN)) {
        seq_pos++;
        seq_pos %= sizeof(sequence);
    }

    if (seq_pos % 4 == 0) {
        gpio_put(TEMPO_LED, 1);
    }
    else {
        gpio_put(TEMPO_LED, 0);
    }

    return true;
}

void button_irq(uint gpio, uint32_t events) {
    switch (gpio) {
    case START_STOP_BTN:
        start_stop_button_handler();
        break;
    case TEST_BTN:
        test_button_handler();
        break;
    case SET_BTN:
        set_button_handler();
        break;
    case ENC_1:
        encoder_handler();
        break;
    }
}

void start_stop_button_handler() {

    if (absolute_time_diff_us(last_startstop_int_time, get_absolute_time()) < 500000) {
        return;
    }

    //clear entire sequence
    if (gpio_get(CLEAR_BTN)) {
        for (int i = 0; i < 16; i++) {
            sequence[i] = 0;
        }
    }
    else {
        if (playing_state == STOPPED) {
            start();
        }
        else {
            stop();
        }
    }

    last_startstop_int_time = get_absolute_time();

}

void stop() {
    playing_state = STOPPED;
    uint8_t note_off[3] = { 0x80 | midi_channel, last_note, 0 };
    tud_midi_stream_write(cable_num, note_off, 3);
    last_note = 0;
}

void start() {
    cancel_repeating_timer(&note_timer);
    seq_pos = 0;
    playing_state = PLAYING;
    add_repeating_timer_ms(bpm_to_delay(120), note_timer_callback, NULL, &note_timer);
}

void test_button_handler() {

    if (absolute_time_diff_us(last_test_int_time, get_absolute_time()) < 125000) {
        return;
    }

    uint8_t note_on[3] = { 0x90 | midi_channel, selected_note, velocity };
    tud_midi_stream_write(cable_num, note_on, 3);

    uint8_t note_off[3] = { 0x80 | midi_channel, selected_note, 0 };
    tud_midi_stream_write(cable_num, note_off, 3);


    last_test_int_time = get_absolute_time();
}

void set_button_handler() {
    if (absolute_time_diff_us(last_set_int_time, get_absolute_time()) < 125000) {
        return;
    }

    if (playing_state != PLAYING) {
        return;
    }


    sequence[seq_pos] = selected_note;


    last_set_int_time = get_absolute_time();
}

void encoder_handler() {
    if (absolute_time_diff_us(last_enc_int_time, get_absolute_time()) < 20000) {
        return;
    }

    if (gpio_get(ENC_2)) {
        if (selected_note > 1)
            selected_note--;
    }
    else {
        if (selected_note < 127) {
            selected_note++;
        }
    }

    last_enc_int_time = get_absolute_time();
}

void scan_inputs(bool* button_states) {
    //latch and clock connected
    gpio_put(COL_DATA, 1);
    gpio_put(COL_CLOCK, 1);
    gpio_put(COL_CLOCK, 0);

    for (int i = 0; i < 16; i++) {
        gpio_put(COL_DATA, 0);
        gpio_put(COL_CLOCK, 1);
        gpio_put(COL_CLOCK, 0);

        sleep_us(10);

        button_states[i] = gpio_get(ROW0);
    }
}

void pattern_button_pressed(uint8_t button) {

    //other modes(eg. sequence selection) can be handled here
    if (gpio_get(CLEAR_BTN)) {
        sequence[button] = 0;
    }
    else {
        sequence[button] = selected_note;
    }
}

void update_display() {
    LCD_clear();
    sleep_ms(1);
    char buffer[16];
    int n = snprintf(buffer, sizeof(buffer), ">%3s(%3d) T:%d", notes[selected_note], selected_note, global_tempo);
    LCD_position(1, 1);
    sleep_ms(1);
    LCD_write_text(buffer, std::min(16, n));
    sleep_ms(1);
    LCD_position(2, 1);
    sleep_ms(1);
    const char* instrument = instrument_mapping[selected_note];
    LCD_write_text((char*)instrument, strlen(instrument));
    sleep_ms(1);
}