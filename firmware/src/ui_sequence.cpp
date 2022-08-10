#include "eeprom_manager.h"
#include "sequence.h"
#include <cstdio>
#include <main.h>
#include <pins.h>
#include <player.h>
#include <ui_sequence.h>
#include <utils.h>

SequenceUI::SequenceUI(EEPROMManager* eeprom_m)
    : eeprom_m(eeprom_m)
{
}

void SequenceUI::update_LED()
{
    std::array<bool, 16> states {};
    if (state == SequenceUIState::CONFIRM) {
        states[selected_sequence] = true;
    }
    update_LEDs(states);
}

void SequenceUI::update_LCD()
{
    if (state == SequenceUIState::CONFIRM) {
        char buffer[16];
        int n = snprintf(buffer, 16, "Seq. slot: %2d", selected_sequence);
        write_to_display(buffer, n, (char*)"SAVE/READ/CANCEL", 16);
    } else {
        write_to_display((char*)"SAVE/READ", 9, (char*)"SEQUENCE", 8);
    }
}

void SequenceUI::pattern_btn_handler(uint8_t button)
{
    selected_sequence = button;
    state = SequenceUIState::CONFIRM;
}

void SequenceUI::enc_handler(bool clockwise)
{
}

void SequenceUI::start_btn_handler()
{
    if (state == SequenceUIState::CONFIRM) {
        eeprom_m->prepare_write(selected_sequence);
        state = SequenceUIState::SELECT;
    }
}

void SequenceUI::setting_btn_handler()
{
}

void SequenceUI::set_btn_handler()
{
    if (state == SequenceUIState::CONFIRM) {
        eeprom_m->prepare_read(selected_sequence);
        state = SequenceUIState::SELECT;
    }
}

void SequenceUI::func_btn_handler()
{
    if (state == SequenceUIState::CONFIRM) {
        state = SequenceUIState::SELECT;
    }
}

void SequenceUI::clear_btn_handler()
{
}

void SequenceUI::hold_btn_handler()
{
}