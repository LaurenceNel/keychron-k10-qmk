// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

/*
 * LED sink pins, three per key row in R, B, G order.
 * Taken from the K2/K3/K4/K8 ports, which all share this list. On the K10,
 * B13/B14/B15 read as transistor bases in the wiring probe, which fits, but
 * the full list and the colour order are not yet confirmed on real hardware.
 */
#define SN32F2XX_RGB_MATRIX_ROW_PINS { C3, C1, C0, C6, C5, C4, C9, C8, C7, C12, C11, C10, B13, C14, C13, B14, B15, D3 }
