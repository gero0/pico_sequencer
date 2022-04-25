#ifndef __MIDI_COMM__
#define __MIDI_COMM__

#include<cstdint>

void MIDI_Init();
void MIDI_usb_note_on(uint8_t note, uint8_t velocity);
void MIDI_usb_note_off(uint8_t note);
void MIDI_note_on(uint8_t note, uint8_t velocity);
void MIDI_note_off(uint8_t note);
int MIDI_usb_recv();

#endif