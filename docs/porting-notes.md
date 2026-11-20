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

1. Count the legs on one side. The SN32F248B is the 64-pin part (16 per side). The 48-pin sibling is the SN32F247B.
2. Put the board in bootloader mode (below) and read its USB ID. `0C45:7040` is the SN32F24xB bootloader. Entering the bootloader changes nothing on the board.

## Bootloader mode

The SN32F248B's bootloader is in ROM and cannot be overwritten. Pulling the BOOT pin (`P2.2`, chip pin 3) low at power-on always enters it.

- Unplug USB and disconnect the battery.
- Bridge the BOOT and GND pads (near the spacebar on Keychron boards) with tweezers, plug in USB while holding them, then let go.
- Keys and lights stay dead while it waits for firmware. Unplug and replug to boot normally.

## Wiring

Found with the probe firmware in `keyboards/keychron/k10/probe`, which drives only the six row pins and reads every `A` and `B` port pin as an input.

| | Pins | Status |
|---|---|---|
| Key rows | `C15, D11, D10, D9, D8, D7` | Confirmed. The same on every Keychron Sonix board |
| Key columns | `A8-A15, B0-B12` (21) | Confirmed |
| Diode direction | `COL2ROW` | Confirmed |
| LED sink pins (R, B, G per row) | `C3, C1, C0, C6, C5, C4, C9, C8, C7, C12, C11, C10, B13, C14, C13, B14, B15, D3` | Taken from the K2/K3/K4/K8 ports. Lights work on the development board; exact row and colour order not individually checked |
| `A0-A7` | No keys. Probably the Bluetooth module (UART2 is on `A0`/`A1`) | Left as inputs |
| Caps Lock / Num Lock indicator LEDs | Unknown | Not driven. The key's own LED turns white instead |
| Win/Mac and Cable/BT/Off switches | Unknown | Ignored |

Each LED column needs its own PWM channel, and `A0-A7` share channels with `B8-B15`, which is why 21 columns land on `A8-A15, B0-B12`.

### Key positions

98 of the 105 keys were pressed during the probe run. Every one sits where the Keychron C2 port puts it, with these differences:

- ISO extras: `#` at `[3,12]`, `\` at `[4,1]`, Enter at `[3,13]`.
- Bottom row, right of space: four keys at `[5,10]` to `[5,13]`.
- Numpad 0 at `[5,17]`, numpad `.` at `[5,19]`.

Seven keys were missed in that run and are placed by inference from the C2 at row 2, columns 14-20: Del, End, PgDn, numpad 7, 8, 9 and +.

## Stock firmware and going back

- There is no known stock firmware for the ISO hot-swap RGB board. Keychron publishes K10 firmware for the soldered Gateron versions only (ANSI RGB V1.05, ISO RGB V1.00) and warns that it can break a hot-swap board.
- The chip is read-protected over USB. Dumping your own stock firmware needs an ST-Link on the SWD pins and SonixQMK's `sonix_dumper`.
- So on this variant, flashing QMK may be one-way. Bricking is unlikely (the bootloader is in ROM), but Bluetooth is gone unless you find the right stock image.

## Settings storage

The default emulated EEPROM on this chip is 1024 bytes. Four VIA layers of a 6x21 matrix need 1008 bytes before anything else, so the VIA keymap uses three layers.

## Not done yet

- ANSI layout variant.
- The dedicated indicator LEDs and the two slide switches.
- Per-key colour control from the host.
- Upstreaming to SonixQMK.
