#ifndef __PLAYER__
#define __PLAYER__

#include "pico/time.h"
#include "sequence.h"

enum class PlayerState {
    STOPPED,
    PLAYING,
};

class Player {
public:
    Player(Player& other) = delete;
    Player(Player&& other) = delete;
    void operator=(const Player&) = delete;
    void operator=(const Player&&) = delete;

    static Player* Instance();


    void start();
    void stop();
    void step();
    void start_stop();
    void init_timer();
    void send_single(uint8_t note);

    void set_tempo(uint8_t tempo);
    uint8_t get_tempo() const;
    void set_global_velocity(uint8_t velocity);
    uint8_t get_global_velocity() const;

    Sequence& get_sequence();
    PlayerState get_state();

private:
    Player();
    ~Player();
    static Player* instance;

    Sequence sequence;
    repeating_timer_t note_timer;
    PlayerState playing_state = PlayerState::STOPPED;
    

    uint8_t tempo = 120;
    uint8_t global_velocity = 127;
    bool tempo_changed = false;

    void send_current_midi_note();
};

#endif