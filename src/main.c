#include <stdio.h>
#include "pico/stdlib.h"

#include "bsp/board.h"
#include "tusb.h"
#include "usb_descriptors.h"

#include "pins.h"

int main() {
    stdio_init_all();
    configure_pins();
    //configure_adc()
    //configure_timers()
    
    //Test MIDI

    //Configure ADC for velocity potentiometer
    //Configure encoder for Note select
    //Timer for display update
    //Timer for playback


    //Expansion ideas: 
    //MIDI Port
    //Saving patterns to eeprom
    //shift registers for input?    

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}

