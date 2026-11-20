// SPDX-License-Identifier: GPL-2.0-or-later

#include "quantum.h"
#include "k10_lighting.h"
#ifdef VIA_ENABLE
#    include "via.h"
#endif

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
 * Win/Mac slide switch on D4, low in the Mac position. It lights the Mac or Windows
 * LED and picks the base layer: K10_MAC_BASE_LAYER or K10_WIN_BASE_LAYER (config.h).
 * A keymap that wants neither can return false from dip_switch_update_user().
 */
static bool mac_mode = false;

#ifdef DIP_SWITCH_ENABLE
bool dip_switch_update_kb(uint8_t index, bool active) {
    if (!dip_switch_update_user(index, active)) {
        return false;
    }
    if (index == 0) {
        mac_mode = active;
        default_layer_set((layer_state_t)1 << (mac_mode ? K10_MAC_BASE_LAYER : K10_WIN_BASE_LAYER));
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

/*
 * The keyboard's own lighting effects (rgb_matrix_kb.inc) and their settings:
 * a per-key painting from a 16-colour palette, and a second colour for the
 * two-colour effects. See k10_lighting.h.
 */
_Static_assert(sizeof(k10_lighting_t) == EECONFIG_KB_DATA_SIZE, "EECONFIG_KB_DATA_SIZE in config.h must equal sizeof(k10_lighting_t)");

// VIA's lighting menu and the painter page pick effects by number. The list is EFFECTS in
// tools/gen_k10_iso.py; this stops the build if the firmware's numbering moves away from it.
_Static_assert(RGB_MATRIX_SOLID_COLOR == 1 && RGB_MATRIX_GRADIENT_LEFT_RIGHT == 2 && RGB_MATRIX_BREATHING == 3 && RGB_MATRIX_CYCLE_ALL == 4 && RGB_MATRIX_CYCLE_LEFT_RIGHT == 5 && RGB_MATRIX_RAINBOW_BEACON == 6 && RGB_MATRIX_RAINDROPS == 7 && RGB_MATRIX_TYPING_HEATMAP == 8 && RGB_MATRIX_SOLID_REACTIVE_SIMPLE == 9 && RGB_MATRIX_SOLID_REACTIVE == 10 && RGB_MATRIX_SPLASH == 11 && RGB_MATRIX_MULTISPLASH == 12 && RGB_MATRIX_CUSTOM_K10_PAINTED == 13 && RGB_MATRIX_CUSTOM_K10_DUAL_WAVE == 14 && RGB_MATRIX_CUSTOM_K10_DUAL_BLEND == 15 && RGB_MATRIX_CUSTOM_K10_DUAL_FADE == 16, "Effect numbers changed: update EFFECTS in tools/gen_k10_iso.py, regenerate, and fix this list");

k10_lighting_t k10_lighting;

// Hue and saturation, 0-255 each. Entry 0 is a placeholder: index 0 means "off".
static const k10_color_t k10_default_palette[K10_PALETTE_SIZE] = {
    {0, 0},     // off
    {234, 230}, // pink
    {128, 255}, // teal
    {0, 0},     // white
    {0, 255},   // red
    {21, 255},  // orange
    {43, 255},  // yellow
    {85, 255},  // green
    {106, 255}, // spring green
    {148, 255}, // sky blue
    {170, 255}, // blue
    {191, 255}, // purple
    {213, 255}, // magenta
    {30, 110},  // warm white
    {185, 130}, // lavender
    {110, 140}, // mint
};

// Default painting: the left half of the board pink, the right half teal.
static void k10_lighting_defaults(void) {
    k10_lighting.version = K10_LIGHTING_VERSION;
    k10_lighting.color2  = k10_default_palette[2];
    memcpy(k10_lighting.palette, k10_default_palette, sizeof(k10_default_palette));
    for (uint8_t i = 0; i < K10_KEY_LEDS; i++) {
        k10_paint_set(i, g_led_config.point[i].x < 112 ? 1 : 2);
    }
}

void keyboard_post_init_kb(void) {
    eeconfig_read_kb_datablock(&k10_lighting, 0, sizeof(k10_lighting));
    if (k10_lighting.version != K10_LIGHTING_VERSION) {
        k10_lighting_defaults();
        eeconfig_update_kb_datablock(&k10_lighting, 0, sizeof(k10_lighting));
    }
    keyboard_post_init_user();
}

#    if defined(VIA_ENABLE) && !defined(K10_PINPROBE)
/*
 * The settings are read and written on VIA's custom channel (channel 0), so VIA's
 * own lighting tab and tools/painter can both edit them. Values:
 */
enum k10_via_value {
    id_k10_color2  = 1, // [hue, sat]
    id_k10_palette = 2, // [index 1-15, hue, sat]
    id_k10_paint   = 3, // [first byte, byte count, bytes...]: the painting, four bits per key
    id_k10_reset   = 4, // set only: back to the default palette and painting
};

// A 32-byte packet, less the command, channel, value id, first byte and byte count.
#        define K10_PAINT_CHUNK_MAX 27

void via_custom_value_command_kb(uint8_t *data, uint8_t length) {
    uint8_t *command_id = &data[0];
    uint8_t  value_id   = data[2];
    uint8_t *value      = &data[3];
    bool     set        = *command_id == id_custom_set_value;

    if (data[1] == id_custom_channel) {
        if (*command_id == id_custom_save) {
            eeconfig_update_kb_datablock(&k10_lighting, 0, sizeof(k10_lighting));
            return;
        }
        if (set || *command_id == id_custom_get_value) {
            switch (value_id) {
                case id_k10_color2:
                    if (set) {
                        k10_lighting.color2.h = value[0];
                        k10_lighting.color2.s = value[1];
                    } else {
                        value[0] = k10_lighting.color2.h;
                        value[1] = k10_lighting.color2.s;
                    }
                    return;
                case id_k10_palette:
                    if (value[0] >= 1 && value[0] < K10_PALETTE_SIZE) {
                        k10_color_t *color = &k10_lighting.palette[value[0]];
                        if (set) {
                            color->h = value[1];
                            color->s = value[2];
                        } else {
                            value[1] = color->h;
                            value[2] = color->s;
                        }
                        return;
                    }
                    break;
                case id_k10_paint:
                    if (value[1] <= K10_PAINT_CHUNK_MAX && value[0] + value[1] <= K10_PAINT_BYTES) {
                        if (set) {
                            memcpy(&k10_lighting.paint[value[0]], &value[2], value[1]);
                        } else {
                            memcpy(&value[2], &k10_lighting.paint[value[0]], value[1]);
                        }
                        return;
                    }
                    break;
                case id_k10_reset:
                    if (set) {
                        k10_lighting_defaults();
                        return;
                    }
                    break;
            }
        }
    }
    *command_id = id_unhandled;
}
#    endif // VIA_ENABLE && !K10_PINPROBE
#endif     // RGB_MATRIX_ENABLE
