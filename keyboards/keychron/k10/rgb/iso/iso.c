// SPDX-License-Identifier: GPL-2.0-or-later

#include "quantum.h"

#ifdef RGB_MATRIX_ENABLE
/*
 * The K10's dedicated Caps Lock / Num Lock indicator pins are not known yet,
 * so show both states by lighting the key's own LED white.
 */
bool rgb_matrix_indicators_advanced_kb(uint8_t led_min, uint8_t led_max) {
    if (!rgb_matrix_indicators_advanced_user(led_min, led_max)) {
        return false;
    }

    led_t state = host_keyboard_led_state();

    if (state.caps_lock) {
        RGB_MATRIX_INDICATOR_SET_COLOR(g_led_config.matrix_co[3][0], 255, 255, 255);
    }
    if (state.num_lock) {
        RGB_MATRIX_INDICATOR_SET_COLOR(g_led_config.matrix_co[1][17], 255, 255, 255);
    }

    return true;
}
#endif // RGB_MATRIX_ENABLE
