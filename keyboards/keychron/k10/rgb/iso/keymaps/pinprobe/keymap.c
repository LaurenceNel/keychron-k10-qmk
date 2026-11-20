// SPDX-License-Identifier: GPL-2.0-or-later
//
// Pin probe for the K10's unmapped pins: the four indicator LEDs at the top right
// (Num Lock, Caps Lock, Mac, Windows) and the two slide switches.
//
// This is the normal VIA keymap plus a small command set on VIA's custom channel,
// driven from the PC by tools/k10_pinprobe.py. The keyboard keeps typing.
//
// Candidates are the pins nothing else uses: A0-A7, D4, D5, D6. They sit as inputs
// with pull-ups. On request the firmware:
//   * reports their levels, with the pull-up on or off;
//   * reports how often each one changed since the last read;
//   * drives ONE pin for up to 3 seconds, then returns it to an input. While a pin
//     is driven, every key goes dark except the F-key with that pin's number
//     (F1 = A0 ... F8 = A7, F9 = D4, F10 = D5, F11 = D6).
//
// It refuses to drive a pin high that reads low under its pull-up, because something
// else is holding that line down.

#include "../via/keymap.c"

static const pin_t probe_pins[] = {A0, A1, A2, A3, A4, A5, A6, A7, D4, D5, D6};
#define PROBE_COUNT (sizeof(probe_pins) / sizeof(probe_pins[0]))

enum probe_values {
    PV_LEVELS  = 1, // get: one byte per pin, level with the pull-up on (2 = being driven)
    PV_TOGGLES = 2, // get: one byte per pin, changes since last read (saturates at 255)
    PV_FLOAT   = 3, // get: one byte per pin, level with the pull-up off
    PV_PULSE   = 4, // set: [pin index, level, duration in 100 ms units (1-30), 0xA5 to allow level 0]
    PV_LEDTEST = 5, // set: [spare LED index, duration in 100 ms units (1-30)]
};

// Second theory for the indicator LEDs: they are not on GPIO pins at all but sit in the
// backlight's own LED matrix, at positions that have no key. Build with K10_PROBE_LEDS=1
// (see tools/gen_k10_iso.py) so every unused position has an LED entry, in this order.
// While one is tested, everything goes dark except that position and one "display" key:
// F1-F12 for the first twelve, then the number keys 1-9.
static const uint8_t spare_leds[][2] = {
    {0, 13}, {0, 17}, {0, 18}, {0, 19}, {0, 20}, {2, 13}, {3, 14}, {3, 15}, {3, 16}, {3, 20}, {4, 12},
    {4, 14}, {4, 16}, {5, 3},  {5, 4},  {5, 5},  {5, 7},  {5, 8},  {5, 9},  {5, 18}, {5, 20},
};
#define SPARE_COUNT (sizeof(spare_leds) / sizeof(spare_leds[0]))

static int8_t   led_test = -1;
static uint32_t led_test_until;

static uint8_t  toggles[PROBE_COUNT];
static uint8_t  last_level[PROBE_COUNT];
static int8_t   pulse_pin = -1;
static uint32_t pulse_until;

void keyboard_post_init_user(void) {
    for (uint8_t i = 0; i < PROBE_COUNT; i++) {
        gpio_set_pin_input_high(probe_pins[i]);
    }
    wait_ms(2);
    for (uint8_t i = 0; i < PROBE_COUNT; i++) {
        last_level[i] = gpio_read_pin(probe_pins[i]);
    }
}

void housekeeping_task_user(void) {
    for (uint8_t i = 0; i < PROBE_COUNT; i++) {
        if (i == pulse_pin) continue;
        uint8_t level = gpio_read_pin(probe_pins[i]);
        if (level != last_level[i]) {
            last_level[i] = level;
            if (toggles[i] < 255) toggles[i]++;
        }
    }
    if (pulse_pin >= 0 && timer_expired32(timer_read32(), pulse_until)) {
        gpio_set_pin_input_high(probe_pins[pulse_pin]);
        wait_us(200);
        last_level[pulse_pin] = gpio_read_pin(probe_pins[pulse_pin]);
        pulse_pin             = -1;
    }
    if (led_test >= 0 && timer_expired32(timer_read32(), led_test_until)) {
        led_test = -1;
    }
}

void via_custom_value_command_kb(uint8_t *data, uint8_t length) {
    uint8_t *command_id = &data[0];
    uint8_t  channel_id = data[1];
    uint8_t  value_id   = data[2];
    uint8_t *value      = &data[3];

    if (channel_id != id_custom_channel) {
        *command_id = id_unhandled;
        return;
    }

    if (*command_id == id_custom_get_value) {
        switch (value_id) {
            case PV_LEVELS:
                for (uint8_t i = 0; i < PROBE_COUNT; i++) {
                    value[i] = (i == pulse_pin) ? 2 : gpio_read_pin(probe_pins[i]);
                }
                return;
            case PV_TOGGLES:
                for (uint8_t i = 0; i < PROBE_COUNT; i++) {
                    value[i]   = toggles[i];
                    toggles[i] = 0;
                }
                return;
            case PV_FLOAT:
                for (uint8_t i = 0; i < PROBE_COUNT; i++) {
                    if (i == pulse_pin) {
                        value[i] = 2;
                        continue;
                    }
                    gpio_set_pin_input(probe_pins[i]);
                    wait_ms(2);
                    value[i] = gpio_read_pin(probe_pins[i]);
                    gpio_set_pin_input_high(probe_pins[i]);
                    wait_us(200);
                    last_level[i] = gpio_read_pin(probe_pins[i]);
                }
                return;
        }
    } else if (*command_id == id_custom_set_value && value_id == PV_PULSE) {
        uint8_t index = value[0], level = value[1], duration = value[2];
        bool    ok    = index < PROBE_COUNT && pulse_pin < 0 && duration >= 1 && duration <= 30;
        if (ok && level && !gpio_read_pin(probe_pins[index])) ok = false; // held low by something else
        if (ok && !level && value[3] != 0xA5) ok = false;                 // driving low needs the explicit flag
        if (!ok) {
            *command_id = id_unhandled;
            return;
        }
        gpio_set_pin_output(probe_pins[index]);
        gpio_write_pin(probe_pins[index], level ? 1 : 0);
        pulse_pin   = index;
        pulse_until = timer_read32() + (uint32_t)duration * 100;
        return;
    } else if (*command_id == id_custom_set_value && value_id == PV_LEDTEST) {
        uint8_t index = value[0], duration = value[1];
        if (index >= SPARE_COUNT || duration < 1 || duration > 30 || pulse_pin >= 0) {
            *command_id = id_unhandled;
            return;
        }
        led_test       = index;
        led_test_until = timer_read32() + (uint32_t)duration * 100;
        return;
    } else if (*command_id == id_custom_save) {
        return;
    }

    *command_id = id_unhandled;
}

// While a pin is driven or a spare LED is tested, show only the key with its number.
bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    if (pulse_pin < 0 && led_test < 0) {
        return true;
    }
    for (uint8_t i = led_min; i < led_max; i++) {
        rgb_matrix_set_color(i, 0, 0, 0);
    }
    if (pulse_pin >= 0) {
        RGB_MATRIX_INDICATOR_SET_COLOR(g_led_config.matrix_co[0][1 + pulse_pin], 255, 255, 255);
    } else {
        // Display key: F1-F12 for 0-11, number keys 1-9 for 12-20.
        uint8_t display = led_test < 12 ? g_led_config.matrix_co[0][1 + led_test] : g_led_config.matrix_co[1][1 + (led_test - 12)];
        uint8_t spare   = g_led_config.matrix_co[spare_leds[led_test][0]][spare_leds[led_test][1]];
        RGB_MATRIX_INDICATOR_SET_COLOR(display, 255, 255, 255);
        if (spare != NO_LED) {
            RGB_MATRIX_INDICATOR_SET_COLOR(spare, 255, 255, 255);
        }
    }
    return false;
}
