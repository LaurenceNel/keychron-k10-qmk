"""Generate the K10 ISO keyboard definition, its VIA definition and the painter's key
layout from one key list.

    python tools/gen_k10_iso.py keyboards/keychron/k10/rgb/iso/keyboard.json keyboards/keychron/k10/rgb/via_json/k10_iso.json tools/painter/layout.js

Matrix positions come from a run of the probe firmware (keyboards/keychron/k10/probe) on a real
board, 2026-11-20. Seven positions (row 2, cols 14-20: Del, End, PgDn, 7, 8, 9, numpad +) were
not pressed during that run and are filled in from the Keychron C2 layout, which every other key
matched. See docs/porting-notes.md.
"""
import json
import os
import pathlib
import sys

out = pathlib.Path(sys.argv[1])

rows = ["C15", "D11", "D10", "D9", "D8", "D7"]
cols = [f"A{i}" for i in range(8, 16)] + [f"B{i}" for i in range(0, 13)]

NAV = 15.25   # x of the nav cluster
PAD = 18.5    # x of the numpad
Y = [0, 1.25, 2.25, 3.25, 4.25, 5.25]

keys = []  # (label, row, col, x, y, w, h)


def k(label, r, c, x, y=None, w=1, h=1):
    keys.append((label, r, c, x, Y[r] if y is None else y, w, h))


# Row 0
k("Esc", 0, 0, 0)
for i, x in enumerate([2, 3, 4, 5, 6.5, 7.5, 8.5, 9.5, 11, 12, 13, 14]):
    k(f"F{i + 1}", 0, i + 1, x)
for i, name in enumerate(["PrtSc", "Key2", "Light"]):
    k(name, 0, 14 + i, NAV + i)

# Row 1
for i, name in enumerate(["`", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "="]):
    k(name, 1, i, i)
k("Bksp", 1, 13, 13, w=2)
for i, name in enumerate(["Ins", "Home", "PgUp"]):
    k(name, 1, 14 + i, NAV + i)
for i, name in enumerate(["NumLk", "P/", "P*", "P-"]):
    k(name, 1, 17 + i, PAD + i)

# Row 2
k("Tab", 2, 0, 0, w=1.5)
for i, name in enumerate(["Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "[", "]"]):
    k(name, 2, 1 + i, 1.5 + i)
for i, name in enumerate(["Del", "End", "PgDn"]):
    k(name, 2, 14 + i, NAV + i)
for i, name in enumerate(["P7", "P8", "P9"]):
    k(name, 2, 17 + i, PAD + i)
k("P+", 2, 20, PAD + 3, h=2)

# Row 3
k("Caps", 3, 0, 0, w=1.75)
for i, name in enumerate(["A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "'"]):
    k(name, 3, 1 + i, 1.75 + i)
k("#", 3, 12, 12.75)
k("Enter", 3, 13, 13.75, y=Y[2], w=1.25, h=2)
for i, name in enumerate(["P4", "P5", "P6"]):
    k(name, 3, 17 + i, PAD + i)

# Row 4
k("LShift", 4, 0, 0, w=1.25)
k("\\", 4, 1, 1.25)
for i, name in enumerate(["Z", "X", "C", "V", "B", "N", "M", ",", ".", "/"]):
    k(name, 4, 2 + i, 2.25 + i)
k("RShift", 4, 13, 12.25, w=2.75)
k("Up", 4, 15, NAV + 1)
for i, name in enumerate(["P1", "P2", "P3"]):
    k(name, 4, 17 + i, PAD + i)
k("PEnter", 4, 20, PAD + 3, h=2)

# Row 5
k("LCtrl", 5, 0, 0, w=1.25)
k("LWin", 5, 1, 1.25, w=1.25)
k("LAlt", 5, 2, 2.5, w=1.25)
k("Space", 5, 6, 3.75, w=6.25)
k("RAlt", 5, 10, 10, w=1.25)
k("RWin", 5, 11, 11.25, w=1.25)
k("Fn", 5, 12, 12.5, w=1.25)
k("RCtrl", 5, 13, 13.75, w=1.25)
for i, name in enumerate(["Left", "Down", "Right"]):
    k(name, 5, 14 + i, NAV + i)
k("P0", 5, 17, PAD, w=2)
k("P.", 5, 19, PAD + 2)

assert len(keys) == 105, len(keys)
assert len({(r, c) for _, r, c, *_ in keys}) == 105

WIDTH, HEIGHT = 22.5, 6.25


def layout_entry(label, r, c, x, y, w, h):
    e = {"label": label, "matrix": [r, c], "x": x, "y": y}
    if w != 1:
        e["w"] = w
    if h != 1:
        e["h"] = h
    return e


def led_entry(label, r, c, x, y, w, h):
    cx = round((x + w / 2) / WIDTH * 224)
    cy = round((y + h / 2) / HEIGHT * 64)
    return {"matrix": [r, c], "x": min(cx, 224), "y": min(cy, 64), "flags": 4}


# The four indicator LEDs at the top right are not on GPIO pins: they sit in the backlight's
# LED matrix at row 0, columns 17-20, which have no key (found with the pinprobe keymap,
# 2026-11-20). They get flag 0, so lighting effects never touch them and only the firmware's
# indicator code in iso.c sets them.
INDICATOR_LEDS = [(0, 17), (0, 18), (0, 19), (0, 20)]  # Num Lock, Caps Lock, Mac, Windows

# With K10_PROBE_LEDS=1 in the environment, every unused matrix position gets an LED entry
# instead, in the order the pinprobe keymap walks them. That build is for finding LEDs on
# another K10 variant; don't commit its keyboard.json.
used = {(r, c) for _, r, c, *_ in keys}
ALL_UNUSED = [(r, c) for r in range(len(rows)) for c in range(len(cols)) if (r, c) not in used]
EXTRA_LEDS = ALL_UNUSED if os.environ.get("K10_PROBE_LEDS") == "1" else INDICATOR_LEDS


# Lighting effects by the firmware's own number: 0 is off, then the animations enabled in
# "rgb_matrix" below, in QMK's order, then the keyboard's own effects from rgb_matrix_kb.inc.
# VIA's stock lighting menu cannot know which effects this firmware enables, nor about the
# keyboard's own, so the VIA definition carries this list instead. iso.c has a _Static_assert
# that fails the build if the firmware's numbering moves away from it.
EFFECTS = [
    "All Off",
    "Solid Colour",
    "Gradient Left/Right",
    "Breathing",
    "Cycle All",
    "Cycle Left/Right",
    "Rainbow Beacon",
    "Raindrops",
    "Typing Heatmap",
    "Reactive Simple",
    "Reactive",
    "Splash",
    "Multisplash",
    "Painted (per key)",
    "Two-Colour Wave",
    "Two-Colour Blend",
    "Two-Colour Fade",
]
PAINTED = EFFECTS.index("Painted (per key)")
TWO_COLOUR = [i for i, name in enumerate(EFFECTS) if name.startswith("Two-Colour")]

# What typing does on top of the background, by enum k10_reaction in k10_lighting.h.
REACTIONS = ["Nothing", "Droplet", "Key Glow", "Wide Glow"]
# The backgrounds a reaction can sit on: those whose colour the firmware works out itself.
REACTIVE = [EFFECTS.index("Solid Colour"), PAINTED] + TWO_COLOUR


def extra_led_entry(r, c):
    return {"matrix": [r, c], "x": round(c * 224 / (len(cols) - 1)), "y": round(r * 64 / (len(rows) - 1)), "flags": 0}


kb = {
    "manufacturer": "Keychron",
    "keyboard_name": "K10 RGB",
    "maintainer": "LaurenceNel",
    "development_board": "sn32f240b_shared_matrix",
    "diode_direction": "COL2ROW",
    "features": {
        "bootmagic": True,
        "command": False,
        "console": False,
        "extrakey": True,
        "mousekey": False,
        "nkro": False,
        "rgb_matrix": True,
        "dip_switch": True,
    },
    # The Win/Mac slide switch. (The Cable/BT switch is on D5 and is not read.)
    "dip_switch": {"pins": ["D4"]},
    "matrix_pins": {"cols": cols, "rows": rows},
    "usb": {"device_version": "1.0.0", "pid": "0xFE13", "vid": "0x3434"},
    "rgb_matrix": {
        "driver": "sn32f2xx",
        "sleep": True,
        "animations": {
            "breathing": True,
            "cycle_all": True,
            "cycle_left_right": True,
            "gradient_left_right": True,
            "rainbow_beacon": True,
            "raindrops": True,
            "typing_heatmap": True,
            "solid_reactive_simple": True,
            "solid_reactive": True,
            "splash": True,
            "multisplash": True,
        },
        "layout": [led_entry(*key) for key in keys] + [extra_led_entry(r, c) for r, c in EXTRA_LEDS],
    },
    "layouts": {"LAYOUT_iso": {"layout": [layout_entry(*key) for key in keys]}},
}

out.parent.mkdir(parents=True, exist_ok=True)
out.write_bytes((json.dumps(kb, indent=4) + "\n").encode("utf-8"))
print("wrote", out, "keys", len(keys), "cols", len(cols))

# Optional second argument: write a VIA (v3) definition for the same layout.
if len(sys.argv) > 2:
    via_out = pathlib.Path(sys.argv[2])
    kle = []
    prev_y = None
    for ry in Y:
        row_keys = sorted((key for key in keys if key[4] == ry), key=lambda key: key[3])
        row, cursor, first = [], 0.0, True
        for label, r, c, x, y, w, h in row_keys:
            props = {}
            if first and prev_y is not None and ry - prev_y != 1:
                props["y"] = round(ry - prev_y - 1, 2)
            if x != cursor:
                props["x"] = round(x - cursor, 2)
            if label == "Enter":
                # ISO Enter: 1.25u wide and 2u tall, with the 1.5u top half.
                props.update({"w": 1.25, "h": 2, "w2": 1.5, "h2": 1, "x2": -0.25})
            else:
                if w != 1:
                    props["w"] = w
                if h != 1:
                    props["h"] = h
            if props:
                row.append(props)
            row.append(f"{r},{c}")
            cursor = x + w
            first = False
        kle.append(row)
        prev_y = ry

    # The lighting tab. Channel 3 is QMK's RGB Matrix (brightness 1, effect 2, speed 3,
    # colour 4). Channel 0 is the keyboard's own (iso.c): value 1 is the second colour,
    # 5 the typing reaction and 6 its colour.
    effect = "{id_qmk_rgb_matrix_effect}"
    reactive = " || ".join(f"{effect} == {i}" for i in REACTIVE)
    lighting = {
        "label": "Lighting",
        "content": [
            {
                "label": "Backlight",
                "content": [
                    {"label": "Brightness", "type": "range", "options": [0, 255], "content": ["id_qmk_rgb_matrix_brightness", 3, 1]},
                    {"label": "Effect", "type": "dropdown", "content": ["id_qmk_rgb_matrix_effect", 3, 2], "options": [[name, i] for i, name in enumerate(EFFECTS)]},
                    {"showIf": f"{effect} != 0", "label": "Effect Speed", "type": "range", "options": [0, 255], "content": ["id_qmk_rgb_matrix_effect_speed", 3, 3]},
                    {"showIf": f"{effect} != 0 && {effect} != {PAINTED}", "label": "Colour", "type": "color", "content": ["id_qmk_rgb_matrix_color", 3, 4]},
                    {"showIf": f"{effect} >= {TWO_COLOUR[0]} && {effect} <= {TWO_COLOUR[-1]}", "label": "Second Colour", "type": "color", "content": ["id_k10_color2", 0, 1]},
                    {"showIf": reactive, "label": "When Typing", "type": "dropdown", "content": ["id_k10_reaction", 0, 5], "options": [[name, i] for i, name in enumerate(REACTIONS)]},
                    {"showIf": f"({reactive}) && {{id_k10_reaction}} != 0", "label": "Typing Colour", "type": "color", "content": ["id_k10_reaction_color", 0, 6]},
                ],
            }
        ],
    }

    via = {
        "name": "Keychron K10 RGB ISO",
        "vendorId": "0x3434",
        "productId": "0xFE13",
        "matrix": {"rows": len(rows), "cols": len(cols)},
        "keycodes": ["qmk_lighting"],
        "menus": [lighting],
        "layouts": {"keymap": kle},
    }
    via_out.parent.mkdir(parents=True, exist_ok=True)
    via_out.write_bytes((json.dumps(via, indent=2) + "\n").encode("utf-8"))
    print("wrote", via_out)

# Optional third argument: the key layout for tools/painter, as a script the page loads.
# Keys are in LED order (LED n is key n), which is what the painting is indexed by.
if len(sys.argv) > 3:
    painter_out = pathlib.Path(sys.argv[3])
    layout = {
        "width": WIDTH,
        "height": HEIGHT,
        "effects": EFFECTS,
        "painted": PAINTED,
        "twoColour": TWO_COLOUR,
        "reactions": REACTIONS,
        "reactive": REACTIVE,
        # ledX and ledY are the LED's position as the firmware has it (0-224, 0-64).
        "keys": [dict(label=label, x=x, y=y, w=w, h=h, ledX=led_entry(label, r, c, x, y, w, h)["x"], ledY=led_entry(label, r, c, x, y, w, h)["y"]) for label, r, c, x, y, w, h in keys],
    }
    painter_out.parent.mkdir(parents=True, exist_ok=True)
    text = "// Generated by tools/gen_k10_iso.py. Do not edit.\nwindow.K10_LAYOUT = " + json.dumps(layout, indent=1) + ";\n"
    painter_out.write_bytes(text.encode("utf-8"))
    print("wrote", painter_out)
