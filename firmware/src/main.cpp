#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
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

#include <player.h>

static repeating_timer_t display_timer;

enum Setting {
    NOTE,
    GL_VELOCITY,
    TEMPO,
};

static enum Setting settings[] = { NOTE, TEMPO, GL_VELOCITY };
static uint8_t current_setting = 0;
uint8_t selected_note = 36;

static absolute_time_t last_startstop_int_time;
static absolute_time_t last_test_int_time;
static absolute_time_t last_set_int_time;
static absolute_time_t last_enc_int_time;
static absolute_time_t last_screen_update_time;
static absolute_time_t last_setting_int_time;

int main()
{
    initialize();
    Player::Instance()->init_timer();

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
    uart_init(uart0, MIDI_BAUDRATE);
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
}

void seq_leds()
{
    for (int i = 0; i < step_count; i++) {
        bool led_state = check_led_state(i);

        gpio_put(SHIFT_DATA, led_state);
        gpio_put(CLOCK, true);
        gpio_put(CLOCK, false);
    }

    gpio_put(LATCH, true);
    gpio_put(LATCH, false);
}

bool check_led_state(int led_id)
{
    auto playing_state = Player::Instance()->get_state();
    auto& sequence = Player::Instance()->get_sequence();

    // light up current step during playing
    bool is_on_position = (playing_state == PlayerState::PLAYING && led_id == sequence.pos);
    bool led_state = false;

    // light up all notes when clear butoon is pressed
    if (!sequence.is_step_clear(led_id) && gpio_get(CLEAR_BTN)) {
        return true;
    }

    if (is_on_position) {
        led_state = true;
    }

    // also light up steps with selected note
    // we want to blink the led when the sequence passes through selected step for
    // better visual feedback
    if (sequence.in_step(selected_note, led_id)) {
        led_state = !is_on_position;
    }

    return led_state;
}

void step_buttons_scan()
{
    // scan twice to debounce
    std::array<bool, step_button_count> button_states = { false, false, false, false, false, false,
        false, false, false, false, false, false,
        false, false, false, false };
    scan_inputs(button_states);

    // detect input only if detected in both scans
    for (int i = 0; i < step_button_count; i++) {
        if (button_states[i]) {
            pattern_button_pressed(i);
        }
    }
}

void scan_inputs(std::array<bool, step_button_count>& button_states)
{
    // latch and clock connected
    gpio_put(COL_DATA, true);
    gpio_put(COL_CLOCK, true);
    gpio_put(COL_CLOCK, false);

    for (int i = 0; i < step_button_count; i++) {
        gpio_put(COL_DATA, false);
        gpio_put(COL_CLOCK, true);
        gpio_put(COL_CLOCK, false);

        sleep_us(10);

        button_states[i] = gpio_get(ROW0);
    }
}

void pattern_button_pressed(uint8_t button)
{
    auto& sequence = Player::Instance()->get_sequence();

    // other modes(eg. sequence selection) can be handled here
    if (gpio_get(CLEAR_BTN)) {
        if (gpio_get(SETTING_BTN)) {
            sequence.clear_step(button);
        } else {
            sequence.clear_step_note(selected_note, button);
        }
    } else if (gpio_get(FUNC_BTN)) {
        if (gpio_get(SETTING_BTN)) {
            sequence.copy_to(button);
        } else {
            sequence.set_part(button);
        }
    } else if (gpio_get(SETTING_BTN)) {
        selected_note = 36 + button;
    } else if (gpio_get(HOLD_BTN)) {
        sequence.pos = button;
    } else {
        sequence.add_to_step(selected_note, button);
    }
}

void button_irq(uint gpio, uint32_t /*events*/)
{
    switch (gpio) {
    case ENC_1:
        encoder_handler();
        break;
    case START_STOP_BTN:
        start_stop_button_handler();
        break;
    case SET_BTN:
        set_button_handler();
        break;
    case SETTING_BTN:
        setting_button_handler();
        break;
    default:
        break;
    }
}

void start_stop_button_handler()
{
    if (absolute_time_diff_us(last_startstop_int_time, get_absolute_time()) < 500000) {
        return;
    }

    auto playing_state = Player::Instance()->get_state();
    auto& sequence = Player::Instance()->get_sequence();

    if (gpio_get(CLEAR_BTN)) {
        sequence.clear_all();
    } else {
        Player::Instance()->start_stop();
    }

    last_startstop_int_time = get_absolute_time();
}

void send_single()
{
    if (absolute_time_diff_us(last_test_int_time, get_absolute_time()) < 125000) {
        return;
    }

    Player::Instance()->send_single(selected_note);

    last_test_int_time = get_absolute_time();
}

void set_button_handler()
{
    if (absolute_time_diff_us(last_set_int_time, get_absolute_time()) < 125000) {
        return;
    }

    auto playing_state = Player::Instance()->get_state();
    auto& sequence = Player::Instance()->get_sequence();

    if (playing_state != PlayerState::PLAYING) {
        return;
    }

    if (gpio_get(FUNC_BTN)) {
        send_single();
    } else {
        sequence.add_to_step(selected_note, sequence.pos);
    }

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

template <typename T>
T change_checked(T val, T lower_bound, T upper_bound, bool increment)
{
    if (increment) {
        if (val < upper_bound) {
            val++;
        }
    } else {
        if (val > lower_bound) {
            val--;
        }
    }

    return val;
}

void change_note(bool increment)
{
    selected_note = change_checked<uint8_t>(selected_note, 1, 127, increment);
}

void change_velocity(bool increment)
{
    auto v = change_checked<uint8_t>(Player::Instance()->get_global_velocity(), 0, 127, increment);
    Player::Instance()->set_global_velocity(v);
}

void change_tempo(bool increment)
{
    auto t = change_checked<uint8_t>(Player::Instance()->get_tempo(), 1, 255, increment);
    Player::Instance()->set_tempo(t);
}

void update_display()
{
    char first_line_buffer[LCD_ROW_LENGTH];
    char second_line_buffer[LCD_ROW_LENGTH];

    int n = format_first_line(first_line_buffer, sizeof(first_line_buffer));
    int n2 = format_second_line(second_line_buffer, sizeof(second_line_buffer));

    write_to_display(first_line_buffer, n, second_line_buffer, n2);
}

int format_first_line(char buffer[], int buflen)
{
    auto& sequence = Player::Instance()->get_sequence();
    auto tempo = Player::Instance()->get_tempo();

    const char* note_name = notes[selected_note];
    int n = snprintf(buffer, buflen, "P:%d %c%3s%cT:%d",
        sequence.current_part + 1,
        settings[current_setting] == NOTE ? '>' : ' ',
        notes[selected_note],
        settings[current_setting] == TEMPO ? '>' : ' ', tempo);

    return n;
}

int format_second_line(char buffer[], int buflen)
{
    auto global_velocity = Player::Instance()->get_global_velocity();

    const char* instrument = instrument_mapping[selected_note];

    int n = snprintf(buffer, buflen, "%9s%cV:%3d", instrument,
        settings[current_setting] == GL_VELOCITY ? '>' : ' ',
        global_velocity);

    return n;
}

void write_to_display(char first_line[], int n1, char second_line[], int n2)
{
    LCD_clear();
    sleep_ms(2);
    LCD_position(1, 1);
    sleep_ms(1);
    LCD_write_text(first_line, std::min(LCD_ROW_LENGTH, n1));
    sleep_ms(1);
    LCD_position(2, 1);
    sleep_ms(1);
    LCD_write_text(second_line, std::min(LCD_ROW_LENGTH, n2));
    sleep_ms(1);
}