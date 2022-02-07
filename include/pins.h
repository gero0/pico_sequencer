#ifndef __PINS__
#define __PINS__

//Reserved
//0,1
//UART for MIDI
//2,3  I2C DAC
//8-11 SPI SD card for settings & samples

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
#define SET_BTN 20
#define TEST_BTN 19
#define CLEAR_BTN 18
#define HOLD_BTN 17
#define SETTING_BTN 16

//Shift register for driving HD44780
#define DISPLAY_DATA 21
#define DISPLAY_CLOCK 22
#define DISPLAY_E 26

//Encoder pins
#define ENC_1 27
#define ENC_2 28

void gpio_configure_pins();
void gpio_set_interrupts(void (*button_irq)(uint, uint32_t));

#endif