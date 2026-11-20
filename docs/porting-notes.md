# Porting notes

How the original Keychron K10 was identified and mapped, and what is still unknown. Useful if you are adding another K10 variant or checking whether your board matches.

## The board this was developed on

| | |
|---|---|
| Model | Keychron K10 (original), ISO, RGB, hot-swap |
| PCB marking | `SFKKBT5003-K10U`, `K-RGBFTCD`, v1.0, dated 2021-06-09 |
| Main chip | Sonix SN32F248B, LQFP64 (16 legs per side). Unmarked on this board |
| Bluetooth | Cypress CYW20730 on a small blue daughterboard |
| USB ID, stock firmware | `05AC:0250` (an Apple ID Keychron reused on its early boards) |
| USB ID, bootloader | `0C45:7040` (Sonix SN32F24xB bootloader) |
| USB ID, this firmware | `3434:FE13` |

This is **not** the "K10 QMK (Version 2)", which ships with QMK on a Westberry WB32F3G71, nor the K10 Pro, Max or HE.

## Confirming the chip without reading it

The chip on this board has no visible marking. Two checks that do not need one:

1. Put the board in bootloader mode (below) and read its USB ID. `0C45:7040` is the SN32F24xB bootloader. Entering the bootloader changes nothing on the board, and it only needs the spacebar keycap off.
2. Count the legs on one side. The SN32F248B is the 64-pin part (16 per side). The 48-pin sibling is the SN32F247B. This one does need the case open.

## Bootloader mode

The SN32F248B's bootloader is in ROM and cannot be overwritten. Pulling the BOOT pin (`P2.2`, chip pin 3) low at power-on always enters it.

- The board has a boot button for this under the spacebar. Pull the spacebar keycap off to reach it; the case stays closed.
- Set the mode switch to Cable and unplug USB, so the board is unpowered.
- Hold the button, plug in USB, then let go.
- Keys and lights stay dead while it waits for firmware. Unplug and replug to boot normally.

## Wiring

Found with the probe firmware in `keyboards/keychron/k10/probe`, which drives only the six row pins and reads every `A` and `B` port pin as an input.

| | Pins | Status |
|---|---|---|
| Key rows | `C15, D11, D10, D9, D8, D7` | Confirmed. The same on every Keychron Sonix board |
| Key columns | `A8-A15, B0-B12` (21) | Confirmed |
| Diode direction | `COL2ROW` | Confirmed |
| LED sink pins (R, B, G per row) | `C3, C1, C0, C6, C5, C4, C9, C8, C7, C12, C11, C10, B13, C14, C13, B14, B15, D3` | Taken from the K2/K3/K4/K8 ports. Lights work on the development board; exact row and colour order not individually checked |
| Indicator LEDs, top right | In the LED matrix, not on GPIO: row 0, columns 17, 18, 19, 20 = Num Lock, Caps Lock, Mac, Windows | Confirmed by lighting each spare matrix position in turn |
| Win/Mac switch | `D4`, low in the Mac position | Confirmed. Lights the Mac or Windows LED and selects the base layer |
| Cable/BT switch | `D5`: low on Cable, high on BT. The middle Off position cuts power, even on USB | Seen, not used |
| `A0`, `A1`, `A3`, `A4` | Bluetooth module lines: they change state once the module has been powered | Left as inputs. Never drive them |
| `A2`, `A5`, `A6`, `A7`, `D6` | No keys, and driving each high lights nothing | Left as inputs |

Each LED column needs its own PWM channel, and `A0-A7` share channels with `B8-B15`, which is why 21 columns land on `A8-A15, B0-B12`.

### Key positions

98 of the 105 keys were pressed during the probe run. Every one sits where the Keychron C2 port puts it, with these differences:

- ISO extras: `#` at `[3,12]`, `\` at `[4,1]`, Enter at `[3,13]`.
- Bottom row, right of space: four keys at `[5,10]` to `[5,13]`.
- Numpad 0 at `[5,17]`, numpad `.` at `[5,19]`.

Seven keys were missed in that run and were placed by inference from the C2 at row 2, columns 14-20: Del, End, PgDn, numpad 7, 8, 9 and +. All seven were then confirmed working on the board with the finished firmware.

### Finding the indicator LEDs

The first guess was spare GPIO pins, as on the K4 and C2. The `pinprobe` keymap and `tools/k10_pinprobe.py` drove `A2`, `A5`, `A6`, `A7` and `D6` high one at a time and no LED lit. The K10 has no need for dedicated pins: its top row leaves four LED-matrix positions free above the numpad, and the indicators are wired there. A build with an LED entry at every unused matrix position (`K10_PROBE_LEDS=1`, `--leds all`) lit them one by one: positions `[0,17]` to `[0,20]` are the four indicators, and none of the other 17 unused positions has an LED.

In `keyboard.json` these four carry LED flag 0, so effects skip them. `iso.c` sets them every frame, and RM_TOGG blanks the keys by clearing the effect flags instead of stopping the matrix, so the indicators keep working with the backlight off.

## Stock firmware and going back

- There is no known stock firmware for the ISO hot-swap RGB board. Keychron publishes K10 firmware for the soldered Gateron versions only (ANSI RGB V1.05, ISO RGB V1.00) and warns that it can break a hot-swap board.
- The chip is read-protected over USB. Dumping your own stock firmware needs an ST-Link on the SWD pins and SonixQMK's `sonix_dumper`.
- So on this variant, flashing QMK may be one-way. Bricking is unlikely (the bootloader is in ROM), but Bluetooth is gone unless you find the right stock image.

## Win/Mac layers

Done the way the other Keychron Sonix ports do it: the slide switch is a QMK DIP switch, and `iso.c` sets the default layer from it. Layer 0 is Windows and layer 2 is Mac (`K10_WIN_BASE_LAYER` and `K10_MAC_BASE_LAYER` in `config.h`), each followed by its Fn layer. A keymap with fewer layers still types in the Mac position, because QMK falls back to layer 0 for keys an active layer does not define.

The Mac layers copy Keychron's own QMK boards: Option and Command swapped, media keys on the top row, F1 to F12 behind Fn. They have not been tried on a Mac yet.

## Settings storage

The default emulated EEPROM on this chip is 1024 bytes. Four VIA layers of a 6x21 matrix need 1008 bytes before anything else, which does not fit, so `config.h` raises it to 1280 (`WEAR_LEVELING_LOGICAL_SIZE`, with twice that as backing flash). That leaves over 200 bytes for VIA macros, the same as three layers had in 1024.

The limit is RAM, not flash: the storage is mirrored in RAM and the chip has 8 KB. After this change the VIA build has 224 bytes of RAM unused, which is not enough for a fifth layer (252 bytes more). Flash is fine: the firmware ends near 38 KB of 64 KB.

## Not done yet

- ANSI layout variant.
- Trying the Mac layout on a Mac.
- Per-key colour control from the host.
- Upstreaming to SonixQMK.
