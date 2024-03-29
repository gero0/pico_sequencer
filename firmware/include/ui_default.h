#ifndef __UI_DEFAULT
#define __UI_DEFAULT

#include "pico/time.h"
#include <array>
#include <ui.h>

enum class Setting {
    NOTE,
    GL_VELOCITY,
    TEMPO,
};

class DefaultUI : public UI {
public:
    void update_LED() override;
    void update_LCD() override;
    void pattern_btn_handler(uint8_t button) override;
    void enc_handler(bool clockwise) override;
    void start_btn_handler() override;
    void setting_btn_handler() override;
    void set_btn_handler() override;
    void func_btn_handler() override;
    void clear_btn_handler() override;
    void hold_btn_handler() override;

private:
    std::array<Setting, 3> settings = { Setting::NOTE, Setting::TEMPO, Setting::GL_VELOCITY };
    uint8_t current_setting = 0;
    uint8_t selected_note = 36;

    bool check_led_state(int led_id);

    int format_first_line(char buffer[], int buflen);
    int format_second_line(char buffer[], int buflen);
};

#endif