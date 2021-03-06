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

#include <player.h>
#include <ui_default.h>

static absolute_time_t last_startstop_int_time;
static absolute_time_t last_test_int_time;
static absolute_time_t last_set_int_time;
static absolute_time_t last_enc_int_time;
static absolute_time_t last_screen_update_time;
static absolute_time_t last_setting_int_time;

static DefaultUI dui;

UI* ui = &dui;

int main()
{
    initialize();
    Player::Instance()->init_timer();

    while (true) {
        tud_task(); // tinyusb device task
        MIDI_usb_recv();
        step_buttons_scan();
        ui->update_LED();
        if (absolute_time_diff_us(last_screen_update_time, get_absolute_time()) > 200000) {
            ui->update_LCD();
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
    LCD_init();
    timers_init();
    ui->update_LED();
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
            ui->pattern_btn_handler(i);
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

    ui->start_btn_handler();

    last_startstop_int_time = get_absolute_time();
}

void set_button_handler()
{
    if (absolute_time_diff_us(last_set_int_time, get_absolute_time()) < 125000) {
        return;
    }

    ui->set_btn_handler();

    last_set_int_time = get_absolute_time();
}

void setting_button_handler()
{

    if (absolute_time_diff_us(last_setting_int_time, get_absolute_time()) < 500000) {
        return;
    }

    ui->setting_btn_handler();

    last_setting_int_time = get_absolute_time();
}

void encoder_handler()
{
    if (absolute_time_diff_us(last_enc_int_time, get_absolute_time()) < 10000) {
        return;
    }

    ui->enc_handler(!gpio_get(ENC_2));

    last_enc_int_time = get_absolute_time();
}
