// SPDX-License-Identifier: GPL-2.0-or-later

// The K10's own lighting effects. Their numbers follow the built-in effects, so the
// order here is fixed: tools/gen_k10_iso.py (EFFECTS) and iso.c rely on it.
RGB_MATRIX_EFFECT(K10_PAINTED)
RGB_MATRIX_EFFECT(K10_DUAL_WAVE)
RGB_MATRIX_EFFECT(K10_DUAL_BLEND)
RGB_MATRIX_EFFECT(K10_DUAL_FADE)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#    include "k10_lighting.h"

// Every key shows the palette colour it was painted with. Index 0 is "off".
static bool K10_PAINTED(effect_params_t *params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        uint8_t index = i < K10_KEY_LEDS ? k10_paint_get(i) : 0;
        if (index == 0) {
            rgb_matrix_set_color(i, 0, 0, 0);
            continue;
        }
        hsv_t hsv = {k10_lighting.palette[index].h, k10_lighting.palette[index].s, rgb_matrix_config.hsv.v};
        rgb_t rgb = rgb_matrix_hsv_to_rgb(hsv);
        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }
    return rgb_matrix_check_finished_leds(led_max);
}

// A colour between the main colour (amount 0) and the second colour (amount 255).
// The hue takes the short way round the colour wheel, so the mix stays vivid
// instead of washing out to grey in the middle.
static rgb_t k10_dual_color(uint8_t amount) {
    hsv_t  hsv  = rgb_matrix_config.hsv;
    int8_t turn = (int8_t)(k10_lighting.color2.h - hsv.h);

    hsv.h = (uint8_t)(hsv.h + ((int16_t)turn * amount) / 256);
    hsv.s = lerp8by8(hsv.s, k10_lighting.color2.s, amount);
    return rgb_matrix_hsv_to_rgb(hsv);
}

static uint8_t k10_effect_time(void) {
    return scale16by8(g_rgb_timer, qadd8(rgb_matrix_config.speed / 4, 1));
}

// The two colours roll across the keyboard from left to right.
static bool K10_DUAL_WAVE(effect_params_t *params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    uint8_t time = k10_effect_time();
    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        rgb_t rgb = k10_dual_color(sin8(g_led_config.point[i].x - time));
        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }
    return rgb_matrix_check_finished_leds(led_max);
}

// Still picture: the main colour at the left edge, the second colour at the right.
static bool K10_DUAL_BLEND(effect_params_t *params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        // x runs 0-224; times 291/256 stretches that to 0-254.
        rgb_t rgb = k10_dual_color(((uint16_t)g_led_config.point[i].x * 291) >> 8);
        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }
    return rgb_matrix_check_finished_leds(led_max);
}

// The whole keyboard drifts from one colour to the other and back.
static bool K10_DUAL_FADE(effect_params_t *params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    rgb_t rgb = k10_dual_color(sin8(k10_effect_time()));
    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }
    return rgb_matrix_check_finished_leds(led_max);
}

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
