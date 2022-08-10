#ifndef __UI_SEQUENCE
#define __UI_SEQUENCE

#include "ui.h"

enum class SequenceUIState {
    SELECT,
    CONFIRM,
};

class SequenceUI : public UI {
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
    bool check_led_state(int led_id);
    SequenceUIState state;
    int selected_sequence = 0;
};

#endif