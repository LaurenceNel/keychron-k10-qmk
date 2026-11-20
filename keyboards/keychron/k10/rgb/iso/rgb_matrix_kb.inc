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

/*
 * The two-colour effects mix the main colour (amount 0) with the second colour
 * (amount 255). The mix is done on red, green and blue, so only those two colours
 * and the direct blend between them ever show. Mixing the hue instead would pass
 * through every colour in between: pink to teal would go by way of purple and blue.
 */
static void k10_dual_colors(rgb_t *first, rgb_t *second) {
    hsv_t hsv = {k10_lighting.color2.h, k10_lighting.color2.s, rgb_matrix_config.hsv.v};
    *first    = rgb_matrix_hsv_to_rgb(rgb_matrix_config.hsv);
    *second   = rgb_matrix_hsv_to_rgb(hsv);
}

static void k10_set_mix(uint8_t led, rgb_t first, rgb_t second, uint8_t amount) {
    rgb_matrix_set_color(led, lerp8by8(first.r, second.r, amount), lerp8by8(first.g, second.g, amount), lerp8by8(first.b, second.b, amount));
}

static uint8_t k10_effect_time(void) {
    return scale16by8(g_rgb_timer, qadd8(rgb_matrix_config.speed / 4, 1));
}

// A sine wave made steeper and then eased, so it sits at one colour or the other for most
// of its length and only blends for a short stretch in between.
static uint8_t k10_band(uint8_t theta) {
    int16_t level = (((int16_t)sin8(theta) - 128) * 5) / 2 + 128;
    return ease8InOutCubic(level < 0 ? 0 : (level > 255 ? 255 : level));
}

// Bands of the two colours roll across the keyboard from left to right.
static bool K10_DUAL_WAVE(effect_params_t *params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    rgb_t first, second;
    k10_dual_colors(&first, &second);
    uint8_t time = k10_effect_time();
    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        // x runs 0-224 and a full wave is 256, so doubling x puts about two bands of each
        // colour on the board.
        k10_set_mix(i, first, second, k10_band((uint8_t)(g_led_config.point[i].x * 2) - time));
    }
    return rgb_matrix_check_finished_leds(led_max);
}

// Still picture: the main colour at the left edge, the second colour at the right.
static bool K10_DUAL_BLEND(effect_params_t *params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    rgb_t first, second;
    k10_dual_colors(&first, &second);
    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        // x runs 0-224; times 291/256 stretches that to 0-254.
        k10_set_mix(i, first, second, ((uint16_t)g_led_config.point[i].x * 291) >> 8);
    }
    return rgb_matrix_check_finished_leds(led_max);
}

// The whole keyboard drifts from one colour to the other and back.
static bool K10_DUAL_FADE(effect_params_t *params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    rgb_t first, second;
    k10_dual_colors(&first, &second);
    uint8_t amount = k10_band(k10_effect_time());
    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        k10_set_mix(i, first, second, amount);
    }
    return rgb_matrix_check_finished_leds(led_max);
}

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
