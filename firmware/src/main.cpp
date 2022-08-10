#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <pico/stdlib.h>

#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/binary_info/code.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include <bsp/board.h>

#include "display.h"
#include "main.h"
#include "midi.h"
#include "pins.h"
#include "utils.h"

#include <player.h>
#include <ui_default.h>
#include <ui_sequence.h>

static absolute_time_t last_startstop_int_time;
static absolute_time_t last_set_int_time;
static absolute_time_t last_enc_int_time;
static absolute_time_t last_screen_update_time;
static absolute_time_t last_setting_int_time;
static absolute_time_t last_func_int_time;
static absolute_time_t last_clear_int_time;
static absolute_time_t last_hold_int_time;

static DefaultUI dui;
static SequenceUI sui;

static UI* UIModes[] = { &dui, &sui };
static uint8_t current_ui_mode = 0;

UI* ui = UIModes[0];

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

void setup_i2c(){
    i2c_init(i2c0, 100 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    bi_decl(bi_2pins_with_func(I2C_SDA, I2C_SCL, GPIO_FUNC_I2C));
}

void initialize()
{
    board_init();
    tusb_init();
    gpio_configure_pins();
    setup_midi_uart();
    gpio_set_interrupts(button_irq);
    setup_i2c();
    LCD_init();
    timers_init();
    ui->update_LED();
}

void timers_init()
{
    last_startstop_int_time = get_absolute_time();
    last_set_int_time = get_absolute_time();
    last_enc_int_time = get_absolute_time();
    last_setting_int_time = get_absolute_time();
    last_screen_update_time = get_absolute_time();
    last_func_int_time = get_absolute_time();
    last_clear_int_time = get_absolute_time();
    last_hold_int_time = get_absolute_time();
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
    case FUNC_BTN:
        func_button_handler();
        break;
    case CLEAR_BTN:
        clear_button_handler();
        break;
    case HOLD_BTN:
        hold_button_handler();
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

void func_button_handler()
{

    if (absolute_time_diff_us(last_func_int_time, get_absolute_time()) < 500000) {
        return;
    }

    ui->func_btn_handler();

    last_func_int_time = get_absolute_time();
}

void clear_button_handler()
{

    if (absolute_time_diff_us(last_clear_int_time, get_absolute_time()) < 500000) {
        return;
    }

    ui->clear_btn_handler();

    last_clear_int_time = get_absolute_time();
}

void hold_button_handler()
{

    if (absolute_time_diff_us(last_hold_int_time, get_absolute_time()) < 500000) {
        return;
    }

    ui->hold_btn_handler();

    last_hold_int_time = get_absolute_time();
}


void encoder_handler()
{
    if (absolute_time_diff_us(last_enc_int_time, get_absolute_time()) < 10000) {
        return;
    }

    bool clockwise = !gpio_get(ENC_2);

    if (gpio_get(SETTING_BTN)) {
        auto i = change_checked<uint8_t>(current_ui_mode, 0, 1, clockwise);
        current_ui_mode = i;
        ui = UIModes[i];
    } else {
        ui->enc_handler(clockwise);
    }

    last_enc_int_time = get_absolute_time();
}
