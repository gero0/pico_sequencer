#ifndef __PINS__
#define __PINS__

//Define button matrix column and row pins

//Free 7,8,9,10,11

#define COL_DATA 4
#define COL_CLOCK 5

#define ROW0 6

//Define LED shift registers pins

#define SHIFT_DATA 12
#define CLOCK 13
#define LATCH 14

//Tempo LED
#define TEMPO_LED 15

//Other buttons
#define CLEAR_BTN 16
#define SET_BTN 17
#define START_STOP_BTN 18
#define TEST_BTN 19

//Encoder pins
#define ENC_1 27
#define ENC_2 28

//Shift register for driving HD44780
#define DISPLAY_DATA 20
#define DISPLAY_CLOCK 21
#define DISPLAY_E 22

//Hold button
#define HOLD_BTN 26


//Reserved
//0,1,2,3
//EEPROM for settings?
//UART for MIDI


void configure_pins();

#endif