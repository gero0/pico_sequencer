#ifndef __UTILS
#define __UTILS

#include <array>

void write_to_display(char first_line[], int n1, char second_line[], int n2);
void update_LEDs(std::array<bool, 16> led_states);

template <typename T>
T change_checked(T val, T lower_bound, T upper_bound, bool increment)
{
    if (increment) {
        if (val < upper_bound) {
            val++;
        }
    } else {
        if (val > lower_bound) {
            val--;
        }
    }

    return val;
}

#endif