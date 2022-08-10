#include <utils.h>
#include <pico/time.h>
#include <algorithm>
#include <display.h>
#include <pins.h>

void write_to_display(char first_line[], int n1, char second_line[], int n2)
{
    LCD_clear();
    sleep_ms(2);
    LCD_position(1, 1);
    sleep_ms(1);
    LCD_write_text(first_line, std::min(LCD_ROW_LENGTH, n1));
    sleep_ms(1);
    LCD_position(2, 1);
    sleep_ms(1);
    LCD_write_text(second_line, std::min(LCD_ROW_LENGTH, n2));
    sleep_ms(1);
}

void update_LEDs(std::array<bool, 16> led_states)
{
    for (int i = 0; i < 16; i++) {
        gpio_put(SHIFT_DATA, led_states[i]);
        gpio_put(CLOCK, true);
        gpio_put(CLOCK, false);
    }

    gpio_put(LATCH, true);
    gpio_put(LATCH, false);
}
