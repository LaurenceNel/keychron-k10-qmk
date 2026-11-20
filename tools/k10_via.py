"""Control the Keychron K10's lighting from the PC over VIA's raw HID protocol.

Needs the VIA build of this firmware on the keyboard and a HID library:
    pip install hidapi        (QMK MSYS's python already has a compatible `hid` module)

    python tools/k10_via.py                      # read-only: protocol version and lighting state
    python tools/k10_via.py --demo               # show solid green, then blue, then restore
    python tools/k10_via.py --color H S          # solid colour, hue and saturation 0-255
    python tools/k10_via.py --color H S --brightness V
    python tools/k10_via.py --effect N           # 0 = off, 1 = solid colour, 2+ = animations

Nothing here writes to the keyboard's saved settings: changes last until it is unplugged.
"""
import argparse
import sys
import time

import hid

VID, PID = 0x3434, 0xFE13
USAGE_PAGE, USAGE = 0xFF60, 0x61

GET_PROTOCOL_VERSION = 0x01
CUSTOM_SET_VALUE = 0x07
CUSTOM_GET_VALUE = 0x08
CH_RGB_MATRIX = 3
V_BRIGHTNESS, V_EFFECT, V_SPEED, V_COLOR = 1, 2, 3, 4
EFFECT_SOLID = 1


def open_device():
    for d in hid.enumerate(VID, PID):
        if d.get("usage_page") == USAGE_PAGE and d.get("usage") == USAGE:
            if hasattr(hid, "Device"):  # pyhidapi
                return hid.Device(path=d["path"])
            dev = hid.device()  # cython-hidapi
            dev.open_path(d["path"])
            return dev
    sys.exit("K10 raw HID interface not found (is the VIA firmware flashed and plugged in?)")


def xfer(dev, payload):
    packet = bytes(payload) + bytes(32 - len(payload))
    dev.write(b"\x00" + packet)  # leading 0 = report ID
    reply = bytes(dev.read(32, 1000))
    if not reply:
        sys.exit("no reply from keyboard")
    return reply


def get(dev, value_id, n=1):
    reply = xfer(dev, [CUSTOM_GET_VALUE, CH_RGB_MATRIX, value_id])
    if reply[0] != CUSTOM_GET_VALUE:
        sys.exit(f"keyboard rejected get value {value_id}: {reply[:6].hex()}")
    return list(reply[3:3 + n])


def put(dev, value_id, *data):
    reply = xfer(dev, [CUSTOM_SET_VALUE, CH_RGB_MATRIX, value_id, *data])
    if reply[0] != CUSTOM_SET_VALUE:
        sys.exit(f"keyboard rejected set value {value_id}: {reply[:6].hex()}")


def state(dev):
    return {
        "brightness": get(dev, V_BRIGHTNESS)[0],
        "effect": get(dev, V_EFFECT)[0],
        "speed": get(dev, V_SPEED)[0],
        "color": get(dev, V_COLOR, 2),
    }


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--demo", action="store_true")
    ap.add_argument("--color", nargs=2, type=int, metavar=("HUE", "SAT"))
    ap.add_argument("--brightness", type=int)
    ap.add_argument("--effect", type=int)
    args = ap.parse_args()

    dev = open_device()
    ver = xfer(dev, [GET_PROTOCOL_VERSION])
    print(f"VIA protocol version: 0x{ver[1]:02X}{ver[2]:02X}")
    before = state(dev)
    print("lighting state:", before)

    changed = False
    if args.color:
        put(dev, V_EFFECT, EFFECT_SOLID)
        put(dev, V_COLOR, args.color[0] & 0xFF, args.color[1] & 0xFF)
        changed = True
    if args.effect is not None:
        put(dev, V_EFFECT, args.effect & 0xFF)
        changed = True
    if args.brightness is not None:
        put(dev, V_BRIGHTNESS, args.brightness & 0xFF)
        changed = True
    if changed:
        print("now:", state(dev))

    if args.demo:
        put(dev, V_EFFECT, EFFECT_SOLID)
        for name, hue in (("green", 85), ("blue", 170)):
            put(dev, V_COLOR, hue, 255)
            print("set solid", name, "->", state(dev))
            time.sleep(1.5)
        put(dev, V_COLOR, *before["color"])
        put(dev, V_SPEED, before["speed"])
        put(dev, V_BRIGHTNESS, before["brightness"])
        put(dev, V_EFFECT, before["effect"])
        print("restored:", state(dev))


if __name__ == "__main__":
    main()
