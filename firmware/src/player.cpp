#include "main.h"
#include "midi.h"
#include "pins.h"
#include "sequence.h"
#include <cstddef>
#include <player.h>

int bpm_to_delay(int bpm)
{
    float seconds_per_beat = 60.0 / bpm;
    float seconds_per_sixteenth = seconds_per_beat / 4;
    float ms_per_sixteenth = seconds_per_sixteenth * 1000;
    return static_cast<int>(ms_per_sixteenth);
}

bool note_timer_callback(struct repeating_timer* /*t*/)
{
    Player::Instance()->step();
    return true;
}

Player* Player::instance = nullptr;

Player::Player(){}

Player* Player::Instance()
{
    if (instance == nullptr) {
        instance = new Player();
    }

    return instance;
}

void Player::init_timer()
{
    add_repeating_timer_ms(bpm_to_delay(tempo), note_timer_callback, nullptr,
        &note_timer);
}

void Player::start()
{
    cancel_repeating_timer(&note_timer);
    sequence.reset();
    playing_state = PlayerState::PLAYING;
    add_repeating_timer_ms(bpm_to_delay(tempo), note_timer_callback, nullptr,
        &note_timer);
}

void Player::stop()
{
    playing_state = PlayerState::STOPPED;
    for (auto& note : sequence.last_notes) {
        if (note != 0) {
            MIDI_usb_note_off(note);
            MIDI_note_off(note);
            note = 0;
        }
    }
}

void Player::start_stop()
{
    if (playing_state == PlayerState::STOPPED) {
        start();
    } else {
        stop();
    }
}

void Player::send_single(uint8_t note)
{
    MIDI_usb_note_on(note, global_velocity);
    MIDI_note_on(note, global_velocity);
    MIDI_usb_note_off(note);
    MIDI_note_off(note);
}

void Player::step()
{
    if (playing_state == PlayerState::PLAYING) {
        send_current_midi_note();
    }

    if (!gpio_get(HOLD_BTN)) {
        sequence.tick(false);
    }

    // blink every 4 steps
    gpio_put(TEMPO_LED, (sequence.pos % 4 == 0));

    if (tempo_changed) {
        tempo_changed = false;
        cancel_repeating_timer(&note_timer);
        add_repeating_timer_ms(bpm_to_delay(tempo), note_timer_callback, nullptr,
            &note_timer);
    }
}

void Player::send_current_midi_note()
{
    for (auto note : sequence.last_notes) {
        if (note != 0) {
            MIDI_usb_note_off(note);
            MIDI_note_off(note);
        }
    }

    auto notes = sequence.current_step();

    for (int i = 0; i < max_notes_per_step; i++) {
        uint8_t note = notes[i];
        if (note != 0) {
            MIDI_usb_note_on(note, global_velocity);
            MIDI_note_on(note, global_velocity);
            sequence.last_notes[i] = note;
        }
    }
}

void Player::set_tempo(uint8_t tempo)
{
    this->tempo = tempo;
    tempo_changed = true;
}

void Player::set_global_velocity(uint8_t velocity)
{
    global_velocity = velocity;
}

PlayerState Player::get_state()
{
    return playing_state;
}

Sequence& Player::get_sequence()
{
    return sequence;
}

uint8_t Player::get_global_velocity() const
{
    return global_velocity;
}

uint8_t Player::get_tempo() const
{
    return tempo;
}