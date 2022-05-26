#ifndef __MAIN_DECLARATIONS__
#define __MAIN_DECLARATIONS__

#include <pico/stdlib.h>
#include <array>

const int step_button_count = 16;
const int MIDI_BAUDRATE = 31250;

int bpm_to_delay(int bpm);
void initialize();
void setup_midi_uart();
void timers_init();
bool check_led_state(int led_id);
void seq_leds();
void display();
void set_interrupts();
void step_buttons_scan();
void scan_inputs(std::array<bool, step_button_count>& button_states);
void pattern_button_pressed(uint8_t button);

void stop();
void start();

void send_current_midi_note();
bool note_timer_callback(struct repeating_timer* t);
void button_irq(uint gpio, uint32_t events);

void send_single();
void start_stop_button_handler();
void set_button_handler();
void encoder_handler();
void setting_button_handler();

void change_note(bool);
void change_velocity(bool);
void change_tempo(bool);

void update_display();
int format_first_line(char* buffer, int buflen);
int format_second_line(char* buffer, int buflen);
void write_to_display(char* first_line, int n1, char* second_line, int n2);

#endif