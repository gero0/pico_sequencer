#ifndef __UI
#define __UI

#include <cstdint>

class UI {
public:
    virtual void update_LED() = 0;
    virtual void update_LCD() = 0;
    virtual void pattern_btn_handler(uint8_t button) = 0;
    virtual void enc_handler(bool clockwise) = 0;
    virtual void start_btn_handler() = 0;
    virtual void setting_btn_handler() = 0;
    virtual void set_btn_handler() = 0;
    virtual void func_btn_handler() = 0;
    virtual void clear_btn_handler() = 0;
    virtual void hold_btn_handler() = 0;
};

#endif