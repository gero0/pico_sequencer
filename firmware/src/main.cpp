#include <algorithm>
#include <cstdio>
#include <pico/stdlib.h>

#include "hardware/gpio.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include <bsp/board.h>

#include "display.h"
#include "main.h"
#include "midi.h"
#include "pins.h"
#include "string_consts.h"

static repeating_timer_t note_timer;
static repeating_timer_t display_timer;

const uint8_t max_notes_per_step = 8;

static uint8_t sequence[16][max_notes_per_step] = {
    { 36, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 36, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 36, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 36, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
};

static uint32_t seq_pos = 0;
static uint8_t last_notes[max_notes_per_step] = { 0, 0, 0, 0 };

static uint32_t global_tempo = 120;
static uint8_t global_velocity = 127;
static uint8_t selected_note = 36;

enum PlayingState {
    STOPPED,
    PLAYING,
};

static enum PlayingState playing_state = STOPPED;

enum Setting {
    NOTE,
    GL_VELOCITY,
    TEMPO,
};

static enum Setting settings[] = { NOTE, TEMPO, GL_VELOCITY };
static uint8_t current_setting = 0;

static absolute_time_t last_startstop_int_time;
static absolute_time_t last_test_int_time;
static absolute_time_t last_set_int_time;
static absolute_time_t last_enc_int_time;
static absolute_time_t last_screen_update_time;
static absolute_time_t last_setting_int_time;

static bool tempo_changed = false;

bool in_sequence(uint8_t note, uint8_t pos)
{
    for (int j = 0; j < max_notes_per_step; j++) {
        if (sequence[pos][j] == note)
            return true;
    }

    return false;
}

bool is_step_clear(uint8_t pos)
{
    for (int j = 0; j < max_notes_per_step; j++) {
        if (sequence[pos][j] != 0)
            return false;
    }

    return true;
}

void clear_step(uint8_t pos)
{
    for (int j = 0; j < max_notes_per_step; j++) {
        sequence[pos][j] = 0;
    }
}

void clear_step_note(uint8_t note, uint8_t pos)
{
    for (int j = 0; j < max_notes_per_step; j++) {
        if (sequence[pos][j] == note) {
            sequence[pos][j] = 0;
        }
    }
}

void add_to_step(uint8_t note, uint8_t pos)
{
    //Don't add the note if it's already present in the step
    if (in_sequence(note, pos))
        return;

    //Assign to first free slot
    for (int j = 0; j < max_notes_per_step; j++) {
        if (sequence[pos][j] == 0) {
            sequence[pos][j] = note;
            return;
        }
    }

    //If there are no more free slots left - too bad! The note won't be added
}

int main()
{
    initialize();

    while (true) {
        tud_task(); // tinyusb device task
        MIDI_usb_recv();
        seq_leds();
        step_buttons_scan();

        if (absolute_time_diff_us(last_screen_update_time, get_absolute_time()) > 200000) {
            update_display();
            last_screen_update_time = get_absolute_time();
        }
    }
}

void setup_midi_uart()
{
    uart_init(uart0, 31250);
    gpio_set_function(MIDI_TX, GPIO_FUNC_UART);
    gpio_set_function(MIDI_RX, GPIO_FUNC_UART);
}

void initialize()
{
    board_init();
    tusb_init();
    gpio_configure_pins();
    setup_midi_uart();
    gpio_set_interrupts(button_irq);
    seq_leds();
    LCD_init();
    timers_init();
}

void timers_init()
{
    last_startstop_int_time = get_absolute_time();
    last_test_int_time = get_absolute_time();
    last_set_int_time = get_absolute_time();
    last_enc_int_time = get_absolute_time();
    last_setting_int_time = get_absolute_time();
    last_screen_update_time = get_absolute_time();

    add_repeating_timer_ms(bpm_to_delay(global_tempo), note_timer_callback, NULL,
        &note_timer);
}

void seq_leds()
{
    for (int i = 0; i < 16; i++) {
        bool led_state = check_led_state(i);

        gpio_put(SHIFT_DATA, led_state);
        gpio_put(CLOCK, 1);
        gpio_put(CLOCK, 0);
    }

    gpio_put(LATCH, 1);
    gpio_put(LATCH, 0);
}

bool check_led_state(int led_id)
{
    // light up current step during playing
    bool is_on_position = (playing_state == PLAYING && led_id == seq_pos);
    bool led_state = false;

    // light up all notes when clear butoon is pressed
    if (!is_step_clear(led_id) && gpio_get(CLEAR_BTN)) {
        return true;
    }

    if (is_on_position)
        led_state = true;

    // also light up steps with selected note
    // we want to blink the led when the sequence passes through selected step for
    // better visual feedback
    if (in_sequence(selected_note, led_id)) {
        led_state = !is_on_position;
    }

    return led_state;
}

void step_buttons_scan()
{
    // scan twice to debounce
    bool button_states[] = { false, false, false, false, false, false,
        false, false, false, false, false, false,
        false, false, false, false };
    scan_inputs(button_states);

    // detect input only if detected in both scans
    for (int i = 0; i < 16; i++) {
        if (button_states[i]) {
            pattern_button_pressed(i);
        }
    }
}

void scan_inputs(bool button_states[])
{
    // latch and clock connected
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

void pattern_button_pressed(uint8_t button)
{
    // other modes(eg. sequence selection) can be handled here
    if (gpio_get(CLEAR_BTN)) {
        if (gpio_get(SETTING_BTN)) {
            clear_step(button);
        } else {
            clear_step_note(selected_note, button);
        }
    } else if (gpio_get(SETTING_BTN)) {
        selected_note = 36 + button;
    } else if (gpio_get(HOLD_BTN)) {
        seq_pos = button;
    } else {
        add_to_step(selected_note, button);
    }
}

int bpm_to_delay(int bpm)
{
    float seconds_per_beat = 60.0 / bpm;
    float seconds_per_sixteenth = seconds_per_beat / 4;
    float ms_per_sixteenth = seconds_per_sixteenth * 1000;
    return (int)ms_per_sixteenth;
}

bool note_timer_callback(struct repeating_timer* t)
{
    if (playing_state == PLAYING) {
        send_current_midi_note();
    }

    if (!gpio_get(HOLD_BTN)) {
        seq_pos++;
        seq_pos %= 16;
    }

    // blink every 4 steps
    gpio_put(TEMPO_LED, (seq_pos % 4 == 0));

    if (tempo_changed) {
        tempo_changed = false;
        cancel_repeating_timer(&note_timer);
        add_repeating_timer_ms(bpm_to_delay(global_tempo), note_timer_callback, NULL,
            &note_timer);
    }

    return true;
}

void send_current_midi_note()
{
    for (auto note : last_notes) {
        if (note != 0) {
            MIDI_usb_note_off(note);
            MIDI_note_off(note);
        }
    }

    uint8_t* notes = sequence[seq_pos];

    for (int i = 0; i < max_notes_per_step; i++) {
        uint8_t note = notes[i];
        if (note != 0) {
            MIDI_usb_note_on(note, global_velocity);
            MIDI_note_on(note, global_velocity);
            last_notes[i] = note;
        }
    }
}

void button_irq(uint gpio, uint32_t events)
{
    switch (gpio) {
    case ENC_1:
        encoder_handler();
        break;
    case START_STOP_BTN:
        start_stop_button_handler();
        break;
    case TEST_BTN:
        test_button_handler();
        break;
    case SET_BTN:
        set_button_handler();
        break;
    case SETTING_BTN:
        setting_button_handler();
    }
}

void start_stop_button_handler()
{
    if (absolute_time_diff_us(last_startstop_int_time, get_absolute_time()) < 500000) {
        return;
    }

    if (gpio_get(CLEAR_BTN)) {
        // clear entire sequence
        for (int i = 0; i < 16; i++) {
            clear_step(i);
        }
    } else {
        if (playing_state == STOPPED)
            start();
        else
            stop();
    }

    last_startstop_int_time = get_absolute_time();
}

void start()
{
    cancel_repeating_timer(&note_timer);
    seq_pos = 0;
    playing_state = PLAYING;
    add_repeating_timer_ms(bpm_to_delay(global_tempo), note_timer_callback, NULL,
        &note_timer);
}

void stop()
{
    playing_state = STOPPED;
    for (auto& note : last_notes) {
        if (note != 0) {
            MIDI_usb_note_off(note);
            MIDI_note_off(note);
            note = 0;
        }
    }
}

void test_button_handler()
{

    if (absolute_time_diff_us(last_test_int_time, get_absolute_time()) < 125000) {
        return;
    }

    MIDI_usb_note_on(selected_note, global_velocity);
    MIDI_note_on(selected_note, global_velocity);
    MIDI_usb_note_off(selected_note);
    MIDI_note_off(selected_note);

    last_test_int_time = get_absolute_time();
}

void set_button_handler()
{
    if (absolute_time_diff_us(last_set_int_time, get_absolute_time()) < 125000) {
        return;
    }

    if (playing_state != PLAYING) {
        return;
    }

    add_to_step(selected_note, seq_pos);

    last_set_int_time = get_absolute_time();
}

void setting_button_handler()
{

    if (absolute_time_diff_us(last_setting_int_time, get_absolute_time()) < 500000) {
        return;
    }

    current_setting++;
    current_setting %= 3;

    last_setting_int_time = get_absolute_time();
}

void encoder_handler()
{
    if (absolute_time_diff_us(last_enc_int_time, get_absolute_time()) < 10000) {
        return;
    }

    switch (settings[current_setting]) {
    case NOTE:
        change_note(gpio_get(ENC_2));
        break;
    case GL_VELOCITY:
        change_velocity(gpio_get(ENC_2));
        break;
    case TEMPO:
        change_tempo(gpio_get(ENC_2));
        break;
    }

    last_enc_int_time = get_absolute_time();
}

void change_note(bool increment)
{
    if (increment) {
        if (selected_note > 1)
            selected_note--;
    } else {
        if (selected_note < 127) {
            selected_note++;
        }
    }
}

void change_velocity(bool increment)
{
    if (increment) {
        if (global_velocity > 0)
            global_velocity--;
    } else {
        if (global_velocity < 127) {
            global_velocity++;
        }
    }
}

void change_tempo(bool increment)
{
    if (increment) {
        if (global_tempo > 1)
            global_tempo--;
    } else {
        if (global_tempo < 255) {
            global_tempo++;
        }
    }

    tempo_changed = true;
}

void update_display()
{
    char first_line_buffer[16];
    char second_line_buffer[16];

    int n = format_first_line(first_line_buffer, sizeof(first_line_buffer));
    int n2 = format_second_line(second_line_buffer, sizeof(second_line_buffer));

    write_to_display(first_line_buffer, n, second_line_buffer, n2);
}

int format_first_line(char* buffer, int buflen)
{
    int n = snprintf(buffer, buflen, "%c%3s(%3d)%cT:%d",
        settings[current_setting] == NOTE ? '>' : ' ',
        notes[selected_note], selected_note,
        settings[current_setting] == TEMPO ? '>' : ' ', global_tempo);

    return n;
}

int format_second_line(char* buffer, int buflen)
{
    const char* instrument = instrument_mapping[selected_note];

    int n = snprintf(buffer, buflen, "%9s%cV:%3d", instrument,
        settings[current_setting] == GL_VELOCITY ? '>' : ' ',
        global_velocity);

    return n;
}

void write_to_display(char* first_line, int n1, char* second_line, int n2)
{
    LCD_clear();
    sleep_ms(2);
    LCD_position(1, 1);
    sleep_ms(1);
    LCD_write_text(first_line, std::min(16, n1));
    sleep_ms(1);
    LCD_position(2, 1);
    sleep_ms(1);
    LCD_write_text(second_line, std::min(16, n2));
    sleep_ms(1);
}