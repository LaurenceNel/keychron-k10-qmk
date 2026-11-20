# QMK for the original Keychron K10

QMK and VIA firmware for the **original Keychron K10** (2021, Sonix SN32F248B), built on [SonixQMK](https://github.com/SonixQMK/qmk_firmware). The original K10 ships with closed firmware: no remapping, no VIA, and no way for software to set the backlight. This gives it all three.

> **Status: early.** It runs on one board (ISO, RGB, hot-swap). Read "Before you flash" first.

## Is this your keyboard?

| | This firmware | Not this firmware |
|---|---|---|
| Model | Keychron K10, the original | K10 QMK (Version 2), K10 Pro, K10 Max, K10 HE: they already run QMK |
| USB ID on stock firmware | `05AC:0250` | `3434:xxxx` |
| Main chip | Sonix SN32F248B (64 pins, may be unmarked) | Westberry WB32, STM32 |
| Layout | ISO | ANSI (not added yet) |
| Backlight | RGB | White (not covered) |
| Switches | Hot-swap | Soldered: untested, see "Before you flash" |

`docs/porting-notes.md` has the PCB markings and how to confirm the chip.

## Before you flash

- **Bluetooth stops working.** The Bluetooth module is a separate closed system. This firmware is USB only.
- **It may be one-way.** Keychron publishes stock K10 firmware for the soldered Gateron versions only. For the ISO hot-swap board there is no known stock image, so there may be no way back to stock (and Bluetooth).
- **Only the hot-swap board is tested.** Keychron ships separate stock firmware for the soldered-switch K10 and warns against putting it on a hot-swap board, so the two may be wired differently. On a soldered board, flash the wiring probe first and compare what it reports with `docs/porting-notes.md`.
- **Bricking is unlikely.** The chip's bootloader is in ROM, so a bad flash can always be redone with the boot button.
- **You don't need to open the case.** The first flash needs the spacebar keycap off, to reach the boot button underneath it. Later flashes need nothing removed.
- This is unofficial and comes with no warranty. It is not affiliated with Keychron.

## What works

- All 105 keys, UK/ISO layout.
- Per-key RGB with QMK's RGB Matrix effects.
- Painting: give every key its own colour from a web page (`tools/painter`). The painting is stored on the keyboard.
- Three two-colour effects (wave, blend, fade) that move between two colours you pick.
- Typing reactions on top of the background: a droplet, a key glow or a wide glow, in their own colour.
- The Win/Mac slide switch: a Windows layout and a Mac layout, each with its own Fn layer.
- The four indicator LEDs at the top right: Num Lock, Caps Lock, and Mac / Windows following the slide switch. They stay on when the backlight is toggled off.
- VIA: remapping, four layers, lighting menu.
- Lighting control from the PC over VIA raw HID (`tools/k10_via.py`).
- Bootloader without opening the case: hold `Esc` while plugging in, or `Fn + Esc`.

## What doesn't (yet)

- Bluetooth (not planned).
- The Mac layout has not been tried on a Mac yet.
- The Cable/BT switch does nothing, except that its middle Off position cuts power.
- ANSI layout.

## Default keymap

The Win/Mac slide switch picks the layout, and the Mac or Windows LED shows which one is active. Flip it at any time; nothing needs replugging.

### Windows

Standard UK ISO. The top row is F1 to F12. The three keys right of F12 are Print Screen, Scroll Lock and "next lighting effect". Bottom row: Ctrl, Win, Alt, space, Alt, Win, Fn, Ctrl.

| Fn + | Does |
|---|---|
| Esc | Bootloader |
| F1 / F2 | Screen brightness down / up |
| F3 / F4 | Task view / File Explorer |
| F5 / F6 | Backlight brightness down / up |
| F7 / F8 / F9 | Previous / play-pause / next |
| F10 / F11 / F12 | Mute / volume down / volume up |
| Print Screen / Scroll Lock | Snipping tool / Pause |
| Light key | Key backlight on / off (the indicator LEDs stay on) |
| Ins / Del | Hue up / down |
| Home / End | Brightness up / down |
| PgUp / PgDn | Saturation up / down |
| Up / Down | Effect speed up / down |
| Left / Right | Previous / next effect |

### Mac

The same keys, with the differences Keychron's own QMK boards make:

- Bottom row: Control, Option, Command, space, Command, Option, Fn, Control.
- The top row does the media and system functions below. Fn + top row gives F1 to F12.
- The key right of F12 takes a screenshot of an area you select (Shift-Command-4); with Fn, of the whole screen (Shift-Command-3). The key after it opens Spotlight (Command-Space).
- Fn + Esc, the light key, and the lighting keys on the navigation cluster and arrows work as on Windows.

| Key | Does |
|---|---|
| F1 / F2 | Screen brightness down / up |
| F3 / F4 | Mission Control / Launchpad |
| F5 / F6 | Backlight brightness down / up |
| F7 / F8 / F9 | Previous / play-pause / next |
| F10 / F11 / F12 | Mute / volume down / volume up |

## Flashing

1. Get `keychron_k10_rgb_iso_via.bin` (or `..._default.bin` without VIA) from the releases, or build it.
2. Enter the bootloader:
   - **From stock firmware:** set the mode switch to Cable and unplug USB. Pull the spacebar keycap off: the boot button is on the circuit board underneath. Hold it down while plugging USB in, then let go. The keys and lights stay dead until you flash.
   - **From this firmware:** hold `Esc` while plugging in, or press `Fn + Esc`.
3. Flash with [Sonix Flasher](https://github.com/SonixQMK/sonix-flasher/releases/latest): the detected `SN32F24xB` device, QMK offset `0x00`, "Flash QMK...".
4. Unplug and replug.

### Using VIA

Open [usevia.app](https://usevia.app) in Chrome or Edge, turn on "Show Design tab" in Settings, and load `keyboards/keychron/k10/rgb/via_json/k10_iso.json` in the Design tab.

VIA shows four layers:

| Layer | Is |
|---|---|
| 0 | Windows |
| 1 | Windows, with Fn held |
| 2 | Mac |
| 3 | Mac, with Fn held |

Remap the pair that matches your switch position. Flashing new firmware resets what you set in VIA.

The Lighting tab's effect list comes from this file, so load the file again after flashing a newer release.

## Painting the keys

`tools/painter/index.html` is a page for colouring keys one by one: halves, thirds, single keys, anything. Open the file in Chrome or Edge, press "Connect keyboard" and pick the K10. It needs the VIA firmware. Nothing is installed and the page loads nothing from the internet.

- Pick one of 15 colours, then click or drag across keys. The crossed-out swatch switches a key off.
- Any of the 15 can be changed to any colour. "Halves", "Thirds" and "Left and right hand" give a quick start.
- The painting is stored on the keyboard: it stays after unplugging and needs no program running. It shows whenever the lighting effect is "Painted (per key)", which the page selects as soon as you paint.
- Profiles: save the current look under a name and load it back later. Profiles are kept in the browser, not on the keyboard, which holds one look at a time. Export them to a file to back them up or move them to another computer.
- "Try without a keyboard" shows the page with nothing connected.

### Background and reaction

The lighting works in two layers, both set from the page or from VIA's Lighting tab.

- **The effect is the background.** Besides QMK's own effects there are Painted and three two-colour effects: Two-Colour Wave, Blend and Fade use the main colour and a second colour, so a wave can be bands of pink and teal instead of a rainbow.
- **"When typing" adds a reaction on top**, in its own colour: Droplet (a ring spreads out from the pressed key), Key Glow (the key lights and fades) or Wide Glow (the key and its neighbours). Speed sets how fast it spreads.

Reactions sit on Solid Colour, Painted and the two-colour effects. A solid teal keyboard with pink droplets is Solid Colour plus Droplet.

The page's picture of the keyboard previews both layers. Type with the page in front and the picture reacts as the keyboard does.

The palette is 15 colours because the chip is nearly out of memory: a free colour for each of 105 keys does not fit.

## Lighting from the PC

```
pip install hidapi
python tools/k10_via.py                  # show the current lighting state
python tools/k10_via.py --color 170 255  # solid blue (hue and saturation, 0-255)
python tools/k10_via.py --demo           # green, then blue, then back
```

Changes are not saved to the keyboard.

## Building

The keyboard folder is an overlay for SonixQMK, tested at `sn32_develop` commit [`c9f3803f`](https://github.com/SonixQMK/qmk_firmware/commit/c9f3803ff1cb910a713c6202cab357268cf98fa6).

```
git clone --recurse-submodules -b sn32_develop https://github.com/SonixQMK/qmk_firmware
# copy this repo's keyboards/keychron/k10 into qmk_firmware/keyboards/keychron/
make keychron/k10/rgb/iso:via
```

On Windows with [QMK MSYS](https://msys.qmk.fm), `scripts\build.ps1` does the copy and builds every target into `dist\`. Keep the firmware checkout on a path without spaces.

`tools/gen_k10_iso.py` generates `keyboard.json` and the VIA definition from one key list. Edit the list there, not the JSON.

## Layout of this repo

```
keyboards/keychron/k10/rgb/iso/     the keyboard: definition, config, default and via keymaps,
                                    plus the pinprobe keymap used to find the indicator LEDs
keyboards/keychron/k10/rgb/via_json VIA definition
keyboards/keychron/k10/probe/       wiring-probe firmware used to map the board
tools/                              layout generator, PC lighting control, pin-probe script
tools/painter/                      the key-painting page
scripts/build.ps1                   Windows build helper
docs/porting-notes.md               hardware facts, how it was mapped, what is unknown
```

## Credits and licence

Built on [SonixQMK](https://github.com/SonixQMK/qmk_firmware) and its Keychron K4 and C2 ports, which this port follows closely. Licensed GPL-2.0-or-later, like QMK. Source for any released binary is this repo plus SonixQMK at the commit above.
