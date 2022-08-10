#include "pico/time.h"
#include "pins.h"
#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <pico/binary_info.h>
#include <pico/stdlib.h>

//PCF8574 pin definitions
#define MASK_RS 1 // P0
#define MASK_RW 2 // P1
#define MASK_E 4 // P2
#define S_BACKLIGHT 3 // P3
#define S_DATA 4 // P4-P7

#define SCREEN_ADDRESS 0x27

void LCD_write_nibble(uint8_t nibble, bool rs)
{
    uint8_t output = (1 << S_BACKLIGHT) | (nibble << S_DATA);

    if (rs) {
        output |= MASK_RS;
    }

    uint8_t output_m = output | MASK_E;

    i2c_write_blocking(i2c0, SCREEN_ADDRESS, &output_m, 1, false);
    i2c_write_blocking(i2c0, SCREEN_ADDRESS, &output, 1, false);
}

void LCD_write_byte(uint8_t byte)
{
    uint8_t low_nibble = byte & 0xF;
    uint8_t high_nibble = (byte >> 4) & 0xF;
    LCD_write_nibble(high_nibble, true);
    LCD_write_nibble(low_nibble, true);
}

void LCD_write_command(uint8_t command)
{
    uint8_t low_nibble = command & 0xF;
    uint8_t high_nibble = (command >> 4) & 0xF;
    LCD_write_nibble(high_nibble, false);
    LCD_write_nibble(low_nibble, false);
}

void LCD_write_text(char* text, uint32_t len)
{
    for (int i = 0; i < len; i++) {
        LCD_write_byte(text[i]);
    }
}

void LCD_clear()
{
    LCD_write_command(1);
}

void LCD_position(uint8_t x, uint8_t y)
{
    int temp = 127 + y;
    if (x == 2)
        temp = temp + 64;
    LCD_write_command(temp);
}

void LCD_write_init_nibble(uint8_t nibble)
{
    uint8_t output = ((nibble >> 4) & 0x0f) << S_DATA;
    uint8_t output_m = (((nibble >> 4) & 0x0f) << S_DATA) | MASK_E;
    i2c_write_blocking(i2c0, SCREEN_ADDRESS, &output_m, 1, false);
    i2c_write_blocking(i2c0, SCREEN_ADDRESS, &output, 1, false);
}

void LCD_init()
{
    sleep_ms(50);
    LCD_write_init_nibble(0x20); // Wake-Up Sequence
    sleep_ms(5);
    LCD_write_init_nibble(0x20);
    sleep_ms(5);
    LCD_write_init_nibble(0x20);
    sleep_ms(5);

    // //4 bit mode
    // LCD_write_init_nibble(0x20);
    // sleep_ms(5);
    LCD_write_command(0x28); // 4-bits, 2 lines, 5x7 font
    sleep_ms(5);
    LCD_write_command(0x0C); // Display ON, No cursors
    sleep_ms(5);
    LCD_write_command(0x06); // Entry mode- Auto-increment, No Display shifting
    sleep_ms(5);
    LCD_write_command(0x01);
    sleep_ms(5);
}