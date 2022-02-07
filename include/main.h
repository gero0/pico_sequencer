#include <pico/stdlib.h>

int MIDI_recv();
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
void setting_button_handler();

void change_note(bool);
void change_velocity(bool);
void change_tempo(bool);