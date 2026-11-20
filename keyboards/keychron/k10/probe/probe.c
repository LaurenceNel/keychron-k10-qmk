// SPDX-License-Identifier: GPL-2.0-or-later
//
// Keychron K10 (original, SN32F248B) wiring probe.
//
// Drives only the six row pins every Keychron Sonix board shares, and reads
// every A/B port pin as a candidate column with a pull-up. Nothing else is
// driven. Each key change is reported on the QMK console as "EV r<row> c<col>
// <1|0>", where col 0-15 = A0-A15 and col 16-31 = B0-B15.
//
// Holding exactly one key for 10 seconds jumps to the bootloader.

#include "quantum.h"
#include "print.h"

static matrix_row_t prev[MATRIX_ROWS];
static matrix_row_t base[MATRIX_ROWS];
static bool         base_set   = false;
static uint32_t     hold_since = 0;
static uint32_t     last_hb    = 0;

void matrix_scan_kb(void) {
    uint32_t now = timer_read32();

    if (!base_set && now > 2000) {
        for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
            base[r] = matrix_get_row(r);
            prev[r] = base[r];
        }
        base_set = true;
    }

    if (base_set) {
        uint8_t new_bits = 0;
        for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
            matrix_row_t cur  = matrix_get_row(r);
            matrix_row_t diff = cur ^ prev[r];
            if (diff) {
                for (uint8_t c = 0; c < MATRIX_COLS; c++) {
                    if (diff & ((matrix_row_t)1 << c)) {
                        uprintf("EV r%u c%u %u\n", r, c, (unsigned)((cur >> c) & 1));
                    }
                }
                prev[r] = cur;
            }
            new_bits += __builtin_popcountl(cur & ~base[r]);
        }

        if (new_bits == 1) {
            if (!hold_since) {
                hold_since = now | 1;
            } else if (now - hold_since > 10000) {
                print("BOOT\n");
                wait_ms(200);
                bootloader_jump();
            }
        } else {
            hold_since = 0;
        }
    }

    if (now - last_hb > 3000) {
        last_hb = now;
        uprintf("HB %lu base", (unsigned long)(now / 1000));
        for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
            uprintf(" %08lX", (unsigned long)base[r]);
        }
        print(" cur");
        for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
            uprintf(" %08lX", (unsigned long)matrix_get_row(r));
        }
        print("\n");
    }

    matrix_scan_user();
}
