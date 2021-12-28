#include <stdio.h>
#include "pico/stdlib.h"

#include "bsp/board.h"
#include "tusb.h"
#include "usb_descriptors.h"

#include "pins.h"
#include "display.h"

int recv_midi();
int bpm_to_delay(int bpm);
void seq_leds();
void display();

static repeating_timer_t note_timer;
static uint8_t midi_channel = 10;
uint8_t const cable_num = 0; // MIDI jack associated with USB endpoint

// Variable that holds the current position in the sequence.
uint32_t seq_pos = 0;

//0 - no note played (why would we want to play 8.18Hz anyway? ;)
//uint8_t sequence[] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
uint8_t sequence[] = { 36,0,42,0,36,0,42,0,36,0,42,0,36,0,42,0 };
uint8_t last_note = 0;

static uint8_t velocity = 127;

enum PlayingState {
    STOPPED,
    PLAYING,
};

static enum PlayingState playing_state = STOPPED;

absolute_time_t last_startstop_int_time;
absolute_time_t last_test_int_time;
absolute_time_t last_set_int_time;

static uint8_t selected_note = 39;
bool send_test = false;

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

void stop() {
    playing_state = STOPPED;
    uint8_t note_off[3] = { 0x80 | midi_channel, last_note, 0};
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

void start_stop_button_handler() {

    if (absolute_time_diff_us(last_startstop_int_time, get_absolute_time()) < 500000) {
        return;
    }

    if (playing_state == STOPPED) {
        start();
    }
    else {
        stop();
    }

    last_startstop_int_time = get_absolute_time();

}

void set_button_handler() {
    if (absolute_time_diff_us(last_set_int_time, get_absolute_time()) < 125000) {
        return;
    }

    if(playing_state != PLAYING){
        return;
    }

    
    sequence[seq_pos] = selected_note;
    

    last_set_int_time = get_absolute_time();
}

bool is_led_on = false;

void button_irq(uint gpio, uint32_t events) {
    if (is_led_on) {
        board_led_off();
        is_led_on = false;
    }
    else {
        board_led_on();
        is_led_on = true;
    }

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
    }
}

int main() {
    board_init();
    tusb_init();
    configure_pins();
    setup_default_uart();

    //Start/stop button irq
    last_startstop_int_time = get_absolute_time();
    last_test_int_time = get_absolute_time();
    last_set_int_time = get_absolute_time();

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

    add_repeating_timer_ms(bpm_to_delay(120), note_timer_callback, NULL, &note_timer);

    //Test MIDI

    //Configure encoder for Note select

    //Expansion ideas: 
    //MIDI Port
    //Saving patterns to eeprom
    //shift registers for input?    

    while (true) {
        tud_task(); // tinyusb device task
        recv_midi();
        seq_leds();
    }
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
