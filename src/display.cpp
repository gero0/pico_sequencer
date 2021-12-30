#include "pins.h"
#include "pico/stdlib.h"

void LCD_write_nibble(uint8_t nibble, bool rs) {
    gpio_put(DISPLAY_E, 1);

    for (int i = 3; i >= 0; i--) {
        bool state = ((nibble >> i) & 1) != 0;
        //bool state = true;
        gpio_put(DISPLAY_DATA, state);
        gpio_put(DISPLAY_CLOCK, 1);
        gpio_put(DISPLAY_CLOCK, 0);
    }

    gpio_put(DISPLAY_DATA, rs);
    gpio_put(DISPLAY_CLOCK, 1);
    gpio_put(DISPLAY_CLOCK, 0);
    //One more pulse(tied clock and latch)
    gpio_put(DISPLAY_CLOCK, 1);
    gpio_put(DISPLAY_CLOCK, 0);

    //execute command
    gpio_put(DISPLAY_E, 0);
    gpio_put(DISPLAY_E, 1);
}

void LCD_write_byte(uint8_t byte) {
    uint8_t low_nibble = byte & 0xF;
    uint8_t high_nibble = byte >> 4;
    LCD_write_nibble(high_nibble, true);
    LCD_write_nibble(low_nibble, true);
}

void LCD_write_command(uint8_t command) {
    uint8_t low_nibble = command & 0xF;
    uint8_t high_nibble = command >> 4;
    LCD_write_nibble(high_nibble, false);
    LCD_write_nibble(low_nibble, false);
}

void LCD_write_text(char* text, uint32_t len) {
    for (int i = 0; i < len; i++) {
        LCD_write_byte(text[i]);
        sleep_ms(1);
    }
}

void LCD_clear() {
    LCD_write_command(1);
}

void LCD_position(uint8_t x, uint8_t y) {
    int temp = 127 + y;
    if (x == 2) temp = temp + 64;
    LCD_write_command(temp);
}

void LCD_init() {
    sleep_ms(50);
    LCD_write_command(0x20); // Wake-Up Sequence
    sleep_ms(50);
    LCD_write_command(0x20);
    sleep_ms(50);
    LCD_write_command(0x20);
    sleep_ms(50);
    LCD_write_command(0x28); // 4-bits, 2 lines, 5x7 font
    sleep_ms(50);
    LCD_write_command(0x0C); // Display ON, No cursors
    sleep_ms(50);
    LCD_write_command(0x06); // Entry mode- Auto-increment, No Display shifting
    sleep_ms(50);
    LCD_write_command(0x01);
    sleep_ms(50);
}