# Keychron K10 RGB (ISO)

The original Keychron K10 (2021, Sonix SN32F248B, PCB `SFKKBT5003-K10U` / `K-RGBFTCD` v1.0), ISO layout, RGB backlight.
This is **not** the "K10 QMK (Version 2)", which ships with QMK on a different chip.

* Hardware Supported: Keychron K10 RGB, ISO, hot-swap (developed on this board)
* Bluetooth is **not** supported. The Bluetooth module is left untouched and the board works over USB only.

Make example for this keyboard (after setting up your build environment):

    make keychron/k10/rgb/iso:default

## Wiring

* Key rows: `C15, D11, D10, D9, D8, D7`
* Key columns: `A8-A15, B0-B12` (21), diodes `COL2ROW`
* Confirmed with the probe firmware in `keyboards/keychron/k10/probe`.
* LED sink pins and the Caps/Num indicator and switch pins are assumed or unknown. See `config.h` and `iso.c`.

## How to flash

1. From stock firmware: short the BOOT pads under the spacebar while plugging in the USB cable.
   From this firmware: hold `Esc` while plugging in (bootmagic), or press `Fn + Esc`.
2. Flash the `.bin` with [Sonix Flasher](https://github.com/SonixQMK/sonix-flasher/releases/latest), device `SN32F24xB`, QMK offset `0x00`.
