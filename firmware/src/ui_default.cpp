#include <display.h>
#include <pins.h>
#include <player.h>
#include <string_consts.h>
#include <ui_default.h>
#include <utils.h>

void DefaultUI::pattern_btn_handler(uint8_t button)
{
    auto& sequence = Player::Instance()->get_sequence();

    // other modes(eg. sequence selection) can be handled here
    if (gpio_get(CLEAR_BTN)) {
        if (gpio_get(SETTING_BTN)) {
            sequence.clear_step(button);
        } else {
            sequence.clear_step_note(selected_note, button);
        }
    } else if (gpio_get(FUNC_BTN)) {
        if (gpio_get(SETTING_BTN)) {
            sequence.copy_to(button);
        } else {
            sequence.set_part(button);
        }
    } else if (gpio_get(SETTING_BTN)) {
        selected_note = 36 + button;
    } else if (gpio_get(HOLD_BTN)) {
        sequence.pos = button;
    } else {
        sequence.add_to_step(selected_note, button);
    }
}

void DefaultUI::enc_handler(bool clockwise)
{
    Setting setting = settings[current_setting];

    switch (setting) {
    case Setting::TEMPO: {
        auto t = change_checked<uint8_t>(Player::Instance()->get_tempo(), 1, 255, clockwise);
        Player::Instance()->set_tempo(t);
        break;
    }

    case Setting::GL_VELOCITY: {
        auto v = change_checked<uint8_t>(Player::Instance()->get_global_velocity(), 0, 127, clockwise);
        Player::Instance()->set_global_velocity(v);
        break;
    }

    case Setting::NOTE: {
        selected_note = change_checked<uint8_t>(selected_note, 1, 127, clockwise);
        break;
    }
    }
}

void DefaultUI::start_btn_handler()
{
    auto playing_state = Player::Instance()->get_state();
    auto& sequence = Player::Instance()->get_sequence();

    if (gpio_get(CLEAR_BTN)) {
        sequence.clear_all();
    } else {
        Player::Instance()->start_stop();
    }
}

void DefaultUI::setting_btn_handler()
{
    current_setting++;
    current_setting %= 3;
}

void DefaultUI::set_btn_handler()
{
    auto playing_state = Player::Instance()->get_state();
    auto& sequence = Player::Instance()->get_sequence();

    if (playing_state != PlayerState::PLAYING) {
        return;
    }

    if (gpio_get(FUNC_BTN)) {
        Player::Instance()->send_single(selected_note);
    } else {
        sequence.add_to_step(selected_note, sequence.pos);
    }
}

bool DefaultUI::check_led_state(int led_id)
{
    auto playing_state = Player::Instance()->get_state();
    auto& sequence = Player::Instance()->get_sequence();

    // light up current step during playing
    bool is_on_position = (playing_state == PlayerState::PLAYING && led_id == sequence.pos);
    bool led_state = false;

    // light up all notes when clear butoon is pressed
    if (!sequence.is_step_clear(led_id) && gpio_get(CLEAR_BTN)) {
        return true;
    }

    if (is_on_position) {
        led_state = true;
    }

    // also light up steps with selected note
    // we want to blink the led when the sequence passes through selected step for
    // better visual feedback
    if (sequence.in_step(selected_note, led_id)) {
        led_state = !is_on_position;
    }

    return led_state;
}

void DefaultUI::update_LED()
{
    std::array<bool, 16> states {};
    for (int i = 0; i < 16; i++) {
        states[i] = check_led_state(i);
    }
    update_LEDs(states);
}

void DefaultUI::update_LCD()
{
    char first_line_buffer[LCD_ROW_LENGTH];
    char second_line_buffer[LCD_ROW_LENGTH];

    int n = format_first_line(first_line_buffer, sizeof(first_line_buffer));
    int n2 = format_second_line(second_line_buffer, sizeof(second_line_buffer));

    write_to_display(first_line_buffer, n, second_line_buffer, n2);
}

int DefaultUI::format_first_line(char buffer[], int buflen)
{
    auto& sequence = Player::Instance()->get_sequence();
    auto tempo = Player::Instance()->get_tempo();

    const char* note_name = notes[selected_note];
    int n = snprintf(buffer, buflen, "P:%d %c%3s%cT:%d",
        sequence.current_part + 1,
        settings[current_setting] == Setting::NOTE ? '>' : ' ',
        notes[selected_note],
        settings[current_setting] == Setting::TEMPO ? '>' : ' ', tempo);

    return n;
}

int DefaultUI::format_second_line(char buffer[], int buflen)
{
    auto global_velocity = Player::Instance()->get_global_velocity();

    const char* instrument = instrument_mapping[selected_note];

    int n = snprintf(buffer, buflen, "%9s%cV:%3d", instrument,
        settings[current_setting] == Setting::GL_VELOCITY ? '>' : ' ',
        global_velocity);

    return n;
}

void DefaultUI::func_btn_handler()
{
}

void DefaultUI::clear_btn_handler()
{
}

void DefaultUI::hold_btn_handler()
{
}