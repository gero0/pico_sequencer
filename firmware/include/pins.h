#ifndef __PINS__
#define __PINS__

#include <cstdint>
#include "pico/stdlib.h"


//UART for MIDI
//8-11 SPI SD card for settings & samples
//26 unused

#define MIDI_TX 0
#define MIDI_RX 1

#define AUDIO_PIN 2

//Step buttons
#define COL_DATA 4
#define COL_CLOCK 5
#define ROW0 6

//Tempo LED
#define TEMPO_LED 7

//Define LED shift registers pins
#define SHIFT_DATA 12
#define CLOCK 13
#define LATCH 14

//Control buttons
#define START_STOP_BTN 15
#define SET_BTN 22
#define TEST_BTN 19
#define CLEAR_BTN 18
#define HOLD_BTN 17
#define SETTING_BTN 16

//HD44780 via i2c
#define DISPLAY_SDA 20
#define DISPLAY_SCL 21

//Encoder pins
#define ENC_1 27
#define ENC_2 28

void gpio_configure_pins();
void gpio_set_interrupts(void (*button_irq)(uint, uint32_t));

#endif