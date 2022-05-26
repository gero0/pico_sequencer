#ifndef _DISPLAY
#define _DISPLAY

#include<stdint.h>

const int LCD_ROW_LENGTH = 16;

void LCD_init();
void LCD_write_command(uint8_t command);
void LCD_write_byte(uint8_t byte);
void LCD_write_text(char* text, uint32_t len);
void LCD_clear();
void LCD_position(uint8_t x, uint8_t y);

#endif