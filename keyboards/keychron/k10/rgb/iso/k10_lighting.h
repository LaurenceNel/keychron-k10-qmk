// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <stdint.h>
#include "util.h"

/*
 * Settings for the keyboard's own lighting effects (rgb_matrix_kb.inc):
 *
 *   Painted            every key shows the palette colour it was painted with
 *   Two-colour effects move between the main colour and a second colour
 *
 * The painting is four bits per key: an index into a 16-entry palette, where
 * index 0 means "off". Free colour per key would need more RAM than this chip
 * has left. The whole struct is stored in the keyboard's settings storage
 * (EECONFIG_KB_DATA_SIZE in config.h) and edited over VIA's custom channel.
 */
#define K10_KEY_LEDS 105 // LEDs 0-104 are the keys, in keyboard.json order
#define K10_PALETTE_SIZE 16
#define K10_PAINT_BYTES ((K10_KEY_LEDS + 1) / 2)
#define K10_LIGHTING_VERSION 1

typedef struct PACKED {
    uint8_t h;
    uint8_t s;
} k10_color_t;

typedef struct PACKED {
    uint8_t     version;
    k10_color_t color2;                    // second colour of the two-colour effects
    k10_color_t palette[K10_PALETTE_SIZE]; // entry 0 is never shown: index 0 is "off"
    uint8_t     paint[K10_PAINT_BYTES];    // palette index per key LED, low nibble first
} k10_lighting_t;

extern k10_lighting_t k10_lighting;

static inline uint8_t k10_paint_get(uint8_t led) {
    uint8_t pair = k10_lighting.paint[led >> 1];
    return (led & 1) ? (pair >> 4) : (pair & 0x0F);
}

static inline void k10_paint_set(uint8_t led, uint8_t index) {
    uint8_t *pair = &k10_lighting.paint[led >> 1];
    *pair         = (led & 1) ? ((*pair & 0x0F) | (index << 4)) : ((*pair & 0xF0) | (index & 0x0F));
}
