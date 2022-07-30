#ifndef __MAIN_DECLARATIONS__
#define __MAIN_DECLARATIONS__

#include <pico/stdlib.h>
#include <array>

const int step_button_count = 16;
const int MIDI_BAUDRATE = 31250;

void initialize();
void setup_midi_uart();
void timers_init();
void step_buttons_scan();
void scan_inputs(std::array<bool, step_button_count>& button_states);
void pattern_button_pressed(uint8_t button);

void button_irq(uint gpio, uint32_t events);
void start_stop_button_handler();
void set_button_handler();
void encoder_handler();
void setting_button_handler();

#endif