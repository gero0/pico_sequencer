#include "midi.h"
#include "tusb.h"

#define MIDI_NOTE_ON 0x90
#define MIDI_NOTE_OFF 0x80

static uint8_t midi_channel = 10;
static uint8_t cable_num = 0; // MIDI jack associated with USB endpoint

void MIDI_Init(uint8_t channel, uint8_t cable_number) {
    midi_channel = channel;
    cable_num = cable_number;
}

void MIDI_usb_note_on(uint8_t note, uint8_t velocity) {
    uint8_t note_on[3] = { MIDI_NOTE_ON | midi_channel, note, velocity };
    tud_midi_stream_write(cable_num, note_on, 3);
}

void MIDI_usb_note_off(uint8_t note) {
    uint8_t note_off[3] = { MIDI_NOTE_OFF | midi_channel, note, 0 };
    tud_midi_stream_write(cable_num, note_off, 3);
}

int MIDI_usb_recv() {
    uint8_t packet[4];
    if (tud_midi_available()) {
        bool result = tud_midi_packet_read(packet);

        return 4;
    }
    else {
        return 0;
    }
}