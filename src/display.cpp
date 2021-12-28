#include "pins.h"
#include "bsp/board.h"

void LCD_write_nibble(uint8_t nibble, bool rs) {
    for (int i = 3; i >= 0; i--) {
        bool state = (bool)(nibble >> i) & 1;
        gpio_put(DISPLAY_DATA, state);
        gpio_put(DISPLAY_CLOCK, 1);
        gpio_put(DISPLAY_CLOCK, 0);
    }

    gpio_put(DISPLAY_DATA, rs)
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
    LCD_write_nibble(low_nibble, true);
    LCD_write_nibble(high_nibble, true);
}

void LCD_write_command(uint8_t command) {
    uint8_t low_nibble = command & 0xF;
    uint8_t high_nibble = command >> 4;
    LCD_write_nibble(low_nibble, false);
    LCD_write_nibble(high_nibble, false);
}

void LCD_write_text(char* text, uint32_t len) {
    for (int i = 0; i < len; i++) {
        LCD_write_byte(text[i])
    }
}

void LCD_clear(){
    LCD_write_command(1);
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