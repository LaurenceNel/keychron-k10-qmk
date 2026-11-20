"""Generate the K10 ISO keyboard definition and its VIA definition from one key list.

    python tools/gen_k10_iso.py keyboards/keychron/k10/rgb/iso/keyboard.json keyboards/keychron/k10/rgb/via_json/k10_iso.json

Matrix positions come from a run of the probe firmware (keyboards/keychron/k10/probe) on a real
board, 2026-11-20. Seven positions (row 2, cols 14-20: Del, End, PgDn, 7, 8, 9, numpad +) were
not pressed during that run and are filled in from the Keychron C2 layout, which every other key
matched. See docs/porting-notes.md.
"""
import json
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
    },
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
        "layout": [led_entry(*key) for key in keys],
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

    via = {
        "name": "Keychron K10 RGB ISO",
        "vendorId": "0x3434",
        "productId": "0xFE13",
        "matrix": {"rows": len(rows), "cols": len(cols)},
        "keycodes": ["qmk_lighting"],
        "menus": ["qmk_rgb_matrix"],
        "layouts": {"keymap": kle},
    }
    via_out.parent.mkdir(parents=True, exist_ok=True)
    via_out.write_bytes((json.dumps(via, indent=2) + "\n").encode("utf-8"))
    print("wrote", via_out)
