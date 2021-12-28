#include "pins.h"
#include "pico/stdlib.h"
#include "bsp/board.h"

void configure_pins(){
    //Matrix

    gpio_init(COL0);
    gpio_set_dir(COL0, GPIO_OUT);

    gpio_init(COL1);
    gpio_set_dir(COL1, GPIO_OUT);

    gpio_init(COL2);
    gpio_set_dir(COL2, GPIO_OUT);

    gpio_init(COL3);
    gpio_set_dir(COL3, GPIO_OUT);

    gpio_init(ROW0);
    gpio_set_dir(ROW0, GPIO_IN);
    gpio_pull_down(ROW0);

    gpio_init(ROW1);
    gpio_set_dir(ROW1, GPIO_IN);
    gpio_pull_down(ROW1);

    gpio_init(ROW2);
    gpio_set_dir(ROW2, GPIO_IN);
    gpio_pull_down(ROW2);

    gpio_init(ROW3);
    gpio_set_dir(ROW3, GPIO_IN);
    gpio_pull_down(ROW3);

    //LED SR
    gpio_init(SHIFT_DATA);
    gpio_set_dir(SHIFT_DATA, GPIO_OUT);

    gpio_init(LATCH);
    gpio_set_dir(LATCH, GPIO_OUT);
    
    gpio_init(CLOCK);
    gpio_set_dir(CLOCK, GPIO_OUT);

    //Tempo LED
    gpio_init(TEMPO_LED);
    gpio_set_dir(TEMPO_LED, GPIO_OUT);

    //Other buttons
    gpio_init(CLEAR_BTN);
    gpio_set_dir(CLEAR_BTN, GPIO_IN);
    gpio_pull_down(CLEAR_BTN);

    gpio_init(SET_BTN);
    gpio_set_dir(SET_BTN, GPIO_IN);
    gpio_pull_down(SET_BTN);

    gpio_init(TEST_BTN);
    gpio_set_dir(TEST_BTN, GPIO_IN);
    gpio_pull_down(TEST_BTN);

    gpio_init(START_STOP_BTN);
    gpio_set_dir(START_STOP_BTN, GPIO_IN);
    gpio_pull_down(START_STOP_BTN);

    gpio_init(HOLD_BTN);
    gpio_set_dir(HOLD_BTN, GPIO_IN);
    gpio_pull_down(HOLD_BTN);

    //Encoder pins
    // ??

    //Display SR
    gpio_init(DISPLAY_DATA);
    gpio_set_dir(DISPLAY_DATA, GPIO_OUT);

    gpio_init(DISPLAY_E);
    gpio_set_dir(DISPLAY_E, GPIO_OUT);
    
    gpio_init(DISPLAY_CLOCK);
    gpio_set_dir(DISPLAY_CLOCK, GPIO_OUT);

}
