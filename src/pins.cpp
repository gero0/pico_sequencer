#include "pins.h"
#include "bsp/board.h"

void gpio_configure_pins(){
    //Inputs

    gpio_init(COL_DATA);
    gpio_set_dir(COL_DATA, GPIO_OUT);

    gpio_init(COL_CLOCK);
    gpio_set_dir(COL_CLOCK, GPIO_OUT);

    gpio_init(ROW0);
    gpio_set_dir(ROW0, GPIO_IN);
    gpio_pull_down(ROW0);

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
    gpio_init(ENC_1);
    gpio_set_dir(ENC_1, GPIO_IN);
    gpio_pull_up(ENC_1);

    gpio_init(ENC_2);
    gpio_set_dir(ENC_2, GPIO_IN);
    gpio_pull_up(ENC_2);

    //Display SR
    gpio_init(DISPLAY_DATA);
    gpio_set_dir(DISPLAY_DATA, GPIO_OUT);

    gpio_init(DISPLAY_E);
    gpio_set_dir(DISPLAY_E, GPIO_OUT);
    gpio_put(DISPLAY_E, 1);
    
    gpio_init(DISPLAY_CLOCK);
    gpio_set_dir(DISPLAY_CLOCK, GPIO_OUT);

}

void gpio_set_interrupts(void (*button_irq)(uint, uint32_t)){
    gpio_set_irq_enabled_with_callback(
        START_STOP_BTN,
        GPIO_IRQ_EDGE_RISE,
        true,
        button_irq
    );

    gpio_set_irq_enabled_with_callback(
        TEST_BTN,
        GPIO_IRQ_EDGE_RISE,
        true,
        button_irq
    );

    gpio_set_irq_enabled_with_callback(
        SET_BTN,
        GPIO_IRQ_EDGE_RISE,
        true,
        button_irq
    );

    gpio_set_irq_enabled_with_callback(
        ENC_1,
        GPIO_IRQ_EDGE_FALL,
        true,
        button_irq
    );

    gpio_set_irq_enabled_with_callback(
        SETTING_BTN,
        GPIO_IRQ_EDGE_RISE,
        true,
        button_irq
    );
}