#include "sequence.h"

Sequence::Sequence()
{
    for (auto& part : parts) {
        part = { {
            { 0, 0, 0, 0 },
            { 0, 0, 0, 0 },
            { 0, 0, 0, 0 },
            { 0, 0, 0, 0 },
            { 0, 0, 0, 0 },
            { 0, 0, 0, 0 },
            { 0, 0, 0, 0 },
            { 0, 0, 0, 0 },
            { 0, 0, 0, 0 },
            { 0, 0, 0, 0 },
            { 0, 0, 0, 0 },
            { 0, 0, 0, 0 },
            { 0, 0, 0, 0 },
            { 0, 0, 0, 0 },
            { 0, 0, 0, 0 },
            { 0, 0, 0, 0 },
        } };
    }
}

void Sequence::tick(bool /*advanceParts*/)
{
    pos++;
    pos %= step_count;
    if (part_change_scheduled && pos == 0) {
        current_part = next_part;
        part_change_scheduled = false;
        next_part = -1;
    }
    // if (advanceParts && pos == 0) {
    //     current_part++;
    //     current_part %= active_parts_count;
    // }
}

void Sequence::reset()
{
    pos = 0;
    current_part = 0;
    part_change_scheduled = false;
    next_part = -1;
}

void Sequence::set_part(int part)
{
    if (current_part == part) {
        return;
    }

    //When we already scheduled a part change, pressing the button again should change part immediately
    if (next_part == part) {
        current_part = next_part;
        part_change_scheduled = false;
    } else {
        if (part < 0 || part >= max_parts) {
            return;
        }
        next_part = part;
        part_change_scheduled = true;
    }
}

void Sequence::copy_to(int part)
{
    if (part < 0 || part >= max_parts) {
        return;
    }

    auto current_part = get_current_part();

    parts[part] = current_part;
}

void Sequence::clear_all()
{
    for (auto& part : parts) {
        for (auto& step : part) {
            for (auto& note : step) {
                note = 0;
            }
        }
    }
}

void Sequence::clear_current_part()
{
    auto& part = get_current_part();
    for (auto& step : part) {
        for (auto& note : step) {
            note = 0;
        }
    }
}

Part& Sequence::get_current_part()
{
    return parts[current_part];
}

std::array<uint8_t, max_notes_per_step> Sequence::current_step()
{
    return parts[current_part][pos];
}

bool Sequence::in_step(uint8_t note, uint8_t pos)
{
    for (int j = 0; j < max_notes_per_step; j++) {
        if (get_current_part()[pos][j] == note) {
            return true;
        }
    }

    return false;
}

bool Sequence::is_step_clear(uint8_t pos)
{
    for (int j = 0; j < max_notes_per_step; j++) {
        if (get_current_part()[pos][j] != 0) {
            return false;
        }
    }

    return true;
}

void Sequence::clear_step(uint8_t pos)
{
    auto& step = get_current_part()[pos];
    for (int j = 0; j < max_notes_per_step; j++) {
        step[j] = 0;
    }
}

void Sequence::clear_step_note(uint8_t note, uint8_t pos)
{
    auto& step = get_current_part()[pos];
    for (int j = 0; j < max_notes_per_step; j++) {
        if (step[j] == note) {
            step[j] = 0;
        }
    }
}

void Sequence::add_to_step(uint8_t note, uint8_t pos)
{
    //Don't add the note if it's already present in the step
    if (in_step(note, pos)) {
        return;
    }

    auto& step = get_current_part()[pos];

    //Assign to first free slot
    for (int j = 0; j < max_notes_per_step; j++) {
        if (step[j] == 0) {
            step[j] = note;
            return;
        }
    }

    //If there are no more free slots left - too bad! The note won't be added
}