#ifndef __SEQUENCE__
#define __SEQUENCE__

#include <array>

const uint8_t max_notes_per_step = 8;
const int max_parts = 4;
const int step_count = 16;

using Part = std::array<std::array<uint8_t, max_notes_per_step>, step_count>;

struct Sequence {
    std::array<Part, max_parts> parts {};
    std::array<uint8_t, max_notes_per_step> last_notes {};

    bool part_change_scheduled = false;
    int next_part = 0;

    int current_part = 0;
    int selected_part = 0;
    int active_parts_count = 4;
    int pos = 0;

    Sequence();

    void tick(bool advanceParts);
    void reset();
    void set_part(int part);
    void copy_to(int part);
    void clear_all();
    void clear_current_part();

    Part& get_current_part();
    std::array<uint8_t, max_notes_per_step> current_step();

    bool in_step(uint8_t note, uint8_t pos);
    bool is_step_clear(uint8_t pos);

    void clear_step(uint8_t pos);
    void clear_step_note(uint8_t note, uint8_t pos);
    void add_to_step(uint8_t note, uint8_t pos);
};

#endif