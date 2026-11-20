// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

/*
 * The Win/Mac slide switch picks the base layer: WIN_BASE or MAC_BASE. Each has its
 * own Fn layer. The switching is done in iso.c, which takes the two layer numbers
 * from K10_WIN_BASE_LAYER and K10_MAC_BASE_LAYER in the keyboard's config.h.
 */
enum layer_names {
    WIN_BASE = 0,
    WIN_FN,
    MAC_BASE,
    MAC_FN,
};

_Static_assert(WIN_BASE == K10_WIN_BASE_LAYER && MAC_BASE == K10_MAC_BASE_LAYER, "Base layers must match K10_WIN_BASE_LAYER and K10_MAC_BASE_LAYER");

#define FN_WIN  MO(WIN_FN)
#define FN_MAC  MO(MAC_FN)

// Windows
#define KC_TASK LGUI(KC_TAB)        // Task view
#define KC_FLXP LGUI(KC_E)          // File Explorer
#define KC_SNIP LGUI(LSFT(KC_S))    // Snipping tool

// macOS
#define KC_SHOT LGUI(LSFT(KC_4))    // Screenshot of a selected area
#define KC_SCRN LGUI(LSFT(KC_3))    // Screenshot of the whole screen
#define KC_SPOT LGUI(KC_SPC)        // Spotlight

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [WIN_BASE] = LAYOUT_iso(
        KC_ESC,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,           KC_PSCR, KC_SCRL, RM_NEXT,
        KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC, KC_INS,  KC_HOME, KC_PGUP, KC_NUM,  KC_PSLS, KC_PAST, KC_PMNS,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC,          KC_DEL,  KC_END,  KC_PGDN, KC_P7,   KC_P8,   KC_P9,   KC_PPLS,
        KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT, KC_NUHS, KC_ENT,                             KC_P4,   KC_P5,   KC_P6,
        KC_LSFT, KC_NUBS, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH,          KC_RSFT,          KC_UP,            KC_P1,   KC_P2,   KC_P3,   KC_PENT,
        KC_LCTL, KC_LGUI, KC_LALT,                            KC_SPC,                             KC_RALT, KC_RGUI, FN_WIN,  KC_RCTL, KC_LEFT, KC_DOWN, KC_RGHT, KC_P0,            KC_PDOT
    ),

    [WIN_FN] = LAYOUT_iso(
        QK_BOOT, KC_BRID, KC_BRIU, KC_TASK, KC_FLXP, RM_VALD, RM_VALU, KC_MPRV, KC_MPLY, KC_MNXT, KC_MUTE, KC_VOLD, KC_VOLU,          KC_SNIP, KC_PAUS, RM_TOGG,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, RM_HUEU, RM_VALU, RM_SATU, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          RM_HUED, RM_VALD, RM_SATD, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,                            _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______,          RM_SPDU,          _______, _______, _______, _______,
        _______, _______, _______,                            _______,                            _______, _______, _______, _______, RM_PREV, RM_SPDD, RM_NEXT, _______,          _______
    ),

    // Mac: Option and Command swap places, and the top row does its printed functions.
    [MAC_BASE] = LAYOUT_iso(
        KC_ESC,  KC_BRID, KC_BRIU, KC_MCTL, KC_LPAD, RM_VALD, RM_VALU, KC_MPRV, KC_MPLY, KC_MNXT, KC_MUTE, KC_VOLD, KC_VOLU,          KC_SHOT, KC_SPOT, RM_NEXT,
        KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC, KC_INS,  KC_HOME, KC_PGUP, KC_NUM,  KC_PSLS, KC_PAST, KC_PMNS,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC,          KC_DEL,  KC_END,  KC_PGDN, KC_P7,   KC_P8,   KC_P9,   KC_PPLS,
        KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT, KC_NUHS, KC_ENT,                             KC_P4,   KC_P5,   KC_P6,
        KC_LSFT, KC_NUBS, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH,          KC_RSFT,          KC_UP,            KC_P1,   KC_P2,   KC_P3,   KC_PENT,
        KC_LCTL, KC_LOPT, KC_LCMD,                            KC_SPC,                             KC_RCMD, KC_ROPT, FN_MAC,  KC_RCTL, KC_LEFT, KC_DOWN, KC_RGHT, KC_P0,            KC_PDOT
    ),

    [MAC_FN] = LAYOUT_iso(
        QK_BOOT, KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,           KC_SCRN, _______, RM_TOGG,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, RM_HUEU, RM_VALU, RM_SATU, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          RM_HUED, RM_VALD, RM_SATD, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,                            _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______,          RM_SPDU,          _______, _______, _______, _______,
        _______, _______, _______,                            _______,                            _______, _______, _______, _______, RM_PREV, RM_SPDD, RM_NEXT, _______,          _______
    ),
};
