// SPDX-License-Identifier: GPL-2.0-or-later

#include "quantum.h"

/*
 * The four indicator LEDs at the top right are part of the RGB matrix: they sit at
 * row 0, columns 17-20, positions that have no key. They carry LED flag 0 in
 * keyboard.json, so lighting effects leave them alone and only this file sets them.
 */
#define K10_LED_NUM_LOCK g_led_config.matrix_co[0][17]
#define K10_LED_CAPS_LOCK g_led_config.matrix_co[0][18]
#define K10_LED_MAC g_led_config.matrix_co[0][19]
#define K10_LED_WIN g_led_config.matrix_co[0][20]

#ifndef K10_INDICATOR_BRIGHTNESS
#    define K10_INDICATOR_BRIGHTNESS 255
#endif

/*
 * Win/Mac slide switch on D4. It only drives the Mac / Windows LEDs for now.
 * Which level means Mac is assumed (low), not yet confirmed on hardware.
 */
static bool mac_mode = false;

#ifdef DIP_SWITCH_ENABLE
bool dip_switch_update_kb(uint8_t index, bool active) {
    if (!dip_switch_update_user(index, active)) {
        return false;
    }
    if (index == 0) {
        mac_mode = active;
    }
    return true;
}
#endif // DIP_SWITCH_ENABLE

#ifdef RGB_MATRIX_ENABLE
static void set_indicator(uint8_t led_min, uint8_t led_max, uint8_t index, bool on) {
    uint8_t level = on ? K10_INDICATOR_BRIGHTNESS : 0;
    RGB_MATRIX_INDICATOR_SET_COLOR(index, level, level, level);
}

bool rgb_matrix_indicators_advanced_kb(uint8_t led_min, uint8_t led_max) {
    if (!rgb_matrix_indicators_advanced_user(led_min, led_max)) {
        return false;
    }

    led_t state = host_keyboard_led_state();

    set_indicator(led_min, led_max, K10_LED_NUM_LOCK, state.num_lock);
    set_indicator(led_min, led_max, K10_LED_CAPS_LOCK, state.caps_lock);
    set_indicator(led_min, led_max, K10_LED_MAC, mac_mode);
    set_indicator(led_min, led_max, K10_LED_WIN, !mac_mode);

    return true;
}

/*
 * The indicators only update while the RGB matrix is running. So "backlight off"
 * (RM_TOGG) blanks the keys by clearing the effect's LED flags instead of stopping
 * the matrix, the same approach Keychron's own QMK boards use.
 */
bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    if (!process_record_user(keycode, record)) {
        return false;
    }
    if (keycode == QK_RGB_MATRIX_TOGGLE) {
        if (record->event.pressed) {
            if (!rgb_matrix_is_enabled()) {
                rgb_matrix_set_flags(LED_FLAG_ALL);
                rgb_matrix_enable();
            } else if (rgb_matrix_get_flags() == LED_FLAG_ALL) {
                rgb_matrix_set_flags(LED_FLAG_NONE);
                rgb_matrix_set_color_all(0, 0, 0);
            } else {
                rgb_matrix_set_flags(LED_FLAG_ALL);
            }
        }
        return false;
    }
    return true;
}
#endif // RGB_MATRIX_ENABLE
