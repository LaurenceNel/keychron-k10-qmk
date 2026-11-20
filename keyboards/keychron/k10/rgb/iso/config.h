// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

/*
 * LED sink pins, three per key row in R, B, G order.
 * Taken from the K2/K3/K4/K8 ports, which all share this list. On the K10,
 * B13/B14/B15 read as transistor bases in the wiring probe, which fits, but
 * the full list and the colour order are not yet confirmed on real hardware.
 */
#define SN32F2XX_RGB_MATRIX_ROW_PINS { C3, C1, C0, C6, C5, C4, C9, C8, C7, C12, C11, C10, B13, C14, C13, B14, B15, D3 }

/*
 * Layers the Win/Mac slide switch selects as the base layer (see iso.c).
 * Each keymap puts its Fn layer straight after the base layer it belongs to.
 */
#define K10_WIN_BASE_LAYER 0
#define K10_MAC_BASE_LAYER 2

/*
 * Settings storage (emulated EEPROM in flash). The default is 1024 bytes, which VIA
 * cannot fit four layers of a 6x21 matrix into: the keymap alone is 1008 bytes.
 * 1280 fits them and leaves room for macros. The whole area is mirrored in RAM and
 * this chip has 8 KB, nearly all of it used, so check the link map before raising
 * it again.
 */
#define WEAR_LEVELING_LOGICAL_SIZE 1280
#define WEAR_LEVELING_BACKING_SIZE 2560
