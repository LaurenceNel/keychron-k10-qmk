"""Find the K10's unmapped pins: the four indicator LEDs and the two slide switches.

Needs the `pinprobe` build on the keyboard (keychron/k10/rgb/iso:pinprobe) and `pip install hidapi`.

    python tools/k10_pinprobe.py            # listen, then walk
    python tools/k10_pinprobe.py --only listen
    python tools/k10_pinprobe.py --only walk --exclude D5,D6
    python tools/k10_pinprobe.py --only walk --pins A2,A5,A6,A7   # drive only these, high
    python tools/k10_pinprobe.py --only walk --low A3,A4     # drive these LOW instead (active-low LEDs)

Listen (30 s): you slide the two switches back and forth; the script notes which pins react.
Walk: each remaining pin is driven high for 3 seconds, one at a time. While a pin is on, the
keyboard goes dark except one F-key (F1 = A0 ... F8 = A7, F9 = D4, F10 = D5, F11 = D6).
Note which F-key is lit when each indicator LED comes on.

A pin that reacted to a switch, showed activity, or is held low is never driven.
"""
import argparse
import json
import pathlib
import sys
import time

import hid

VID, PID = 0x3434, 0xFE13
USAGE_PAGE, USAGE = 0xFF60, 0x61

SET, GET, UNHANDLED = 0x07, 0x08, 0xFF
CH_CUSTOM, CH_RGB = 0, 3
PV_LEVELS, PV_TOGGLES, PV_FLOAT, PV_PULSE = 1, 2, 3, 4
RGB_BRIGHTNESS, RGB_EFFECT = 1, 2

PINS = ["A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7", "D4", "D5", "D6"]
FKEY = {p: f"F{i + 1}" for i, p in enumerate(PINS)}


def open_device():
    for d in hid.enumerate(VID, PID):
        if d.get("usage_page") == USAGE_PAGE and d.get("usage") == USAGE:
            if hasattr(hid, "Device"):  # pyhidapi
                return hid.Device(path=d["path"])
            dev = hid.device()  # cython-hidapi
            dev.open_path(d["path"])
            return dev
    sys.exit("K10 raw HID interface not found. Is the pinprobe firmware flashed and plugged in?")


def xfer(dev, payload):
    dev.write(b"\x00" + bytes(payload) + bytes(32 - len(payload)))
    reply = bytes(dev.read(32, 1000))
    if not reply:
        sys.exit("no reply from keyboard")
    return reply


def probe_get(dev, value_id):
    reply = xfer(dev, [GET, CH_CUSTOM, value_id])
    if reply[0] == UNHANDLED:
        sys.exit("The keyboard is not running the pinprobe firmware (it rejected the probe command).")
    return list(reply[3 : 3 + len(PINS)])


def pulse(dev, pin, level, tenths):
    reply = xfer(dev, [SET, CH_CUSTOM, PV_PULSE, PINS.index(pin), 1 if level else 0, tenths, 0 if level else 0xA5])
    return reply[0] != UNHANDLED


def rgb_get(dev, value_id):
    return xfer(dev, [GET, CH_RGB, value_id])[3]


def rgb_put(dev, value_id, value):
    xfer(dev, [SET, CH_RGB, value_id, value])


def listen(dev, seconds):
    print(f"\nLISTEN, {seconds} seconds.")
    print("  Slide the Win/Mac switch back and forth a few times, then the Cable/BT switch.")
    print("  Leave each position for about a second. Don't press keys.\n")
    seen_up = {p: set() for p in PINS}
    seen_float = {p: set() for p in PINS}
    toggles = dict.fromkeys(PINS, 0)
    events = []
    probe_get(dev, PV_TOGGLES)  # clear the counters
    last = None
    start = time.monotonic()
    n = 0
    lost = False
    try:
        while (now := time.monotonic() - start) < seconds:
            up = probe_get(dev, PV_LEVELS)
            for p, v in zip(PINS, up):
                seen_up[p].add(v)
            if last is not None:
                for p, a, b in zip(PINS, last, up):
                    if a != b:
                        events.append({"t": round(now, 1), "pin": p, "from": a, "to": b})
                        print(f"  {now:5.1f}s  {p} went {a} -> {b}")
            last = up
            if n % 4 == 0:
                for p, v in zip(PINS, probe_get(dev, PV_FLOAT)):
                    seen_float[p].add(v)
                for p, v in zip(PINS, probe_get(dev, PV_TOGGLES)):
                    toggles[p] += v
            n += 1
            time.sleep(0.15)
        for p, v in zip(PINS, probe_get(dev, PV_TOGGLES)):
            toggles[p] += v
    except OSError:
        # On the K10 the mode switch passes through Off between Cable and BT, which cuts power.
        lost = True
        print(f"  {time.monotonic() - start:5.1f}s  keyboard lost power or was unplugged; keeping what was seen so far")

    result = {}
    for p in PINS:
        if seen_up[p] == {0}:
            kind = "held low"
        elif len(seen_up[p]) > 1 or toggles[p]:
            kind = "reacts (switch or busy line)"
        elif len(seen_float[p]) > 1:
            kind = "quiet (wanders with the pull-up off)"
        else:
            kind = "quiet"
        result[p] = {"kind": kind, "pullup_levels": sorted(seen_up[p]), "float_levels": sorted(seen_float[p]), "changes": toggles[p]}
    print("\n  pin   pull-up  floating  changes  verdict")
    for p in PINS:
        r = result[p]
        print(f"  {p:<4}  {str(r['pullup_levels']):<7}  {str(r['float_levels']):<8}  {r['changes']:<7}  {r['kind']}")
    return result, events, lost


def walk(dev, pins, level, seconds):
    word = "HIGH" if level else "LOW"
    print(f"\nWALK: driving {len(pins)} pin(s) {word}, {seconds:g} s each: {', '.join(pins)}")
    print("  Watch the four LEDs at the top right. One F-key is lit while each pin is on.")
    print("  Note which F-key is lit when each LED comes on.\n")
    brightness, effect = rgb_get(dev, RGB_BRIGHTNESS), rgb_get(dev, RGB_EFFECT)
    if effect == 0:
        rgb_put(dev, RGB_EFFECT, 1)
    if brightness < 80:
        rgb_put(dev, RGB_BRIGHTNESS, 120)
    for i in (3, 2, 1):
        print(f"  starting in {i}...")
        time.sleep(1)
    driven = []
    try:
        for p in pins:
            if pulse(dev, p, level, round(seconds * 10)):
                print(f"  {FKEY[p]:<4} lit   pin {p} driven {word}")
                driven.append(p)
                time.sleep(seconds + 1.0)
            else:
                print(f"  {FKEY[p]:<4} --    pin {p} skipped: the keyboard refused (something holds it low)")
    finally:
        time.sleep(0.5)
        rgb_put(dev, RGB_BRIGHTNESS, brightness)
        rgb_put(dev, RGB_EFFECT, effect)
    return driven


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--only", choices=["listen", "walk"])
    ap.add_argument("--listen-seconds", type=float, default=30)
    ap.add_argument("--pulse-seconds", type=float, default=3.0)
    ap.add_argument("--exclude", default="", help="pins never to drive, e.g. D5,D6")
    ap.add_argument("--pins", default="", help="walk ONLY these pins, driving them high, e.g. A2,A5,A6,A7")
    ap.add_argument("--low", default="", help="walk ONLY these pins, driving them LOW (for active-low LEDs)")
    ap.add_argument("--out", default=str(pathlib.Path(__file__).resolve().parent.parent / "dist" / "pinprobe-result.json"))
    args = ap.parse_args()
    if not 0.1 <= args.pulse_seconds <= 3.0:
        sys.exit("--pulse-seconds must be between 0.1 and 3")
    exclude = {p.strip().upper() for p in args.exclude.split(",") if p.strip()}
    low = [p.strip().upper() for p in args.low.split(",") if p.strip()]
    only_pins = [p.strip().upper() for p in args.pins.split(",") if p.strip()]
    for p in exclude | set(low) | set(only_pins):
        if p not in PINS:
            sys.exit(f"unknown pin {p}; choose from {', '.join(PINS)}")

    dev = open_device()
    out = {"pins": PINS, "fkeys": FKEY}
    start_up, start_float = probe_get(dev, PV_LEVELS), probe_get(dev, PV_FLOAT)
    out["start"] = {p: {"pullup": u, "float": f} for p, u, f in zip(PINS, start_up, start_float)}
    print("Levels now (pull-up on / off):  " + "  ".join(f"{p}={u}/{f}" for p, u, f in zip(PINS, start_up, start_float)))

    verdicts, lost = {}, False
    if args.only != "walk":
        verdicts, events, lost = listen(dev, args.listen_seconds)
        out["listen"], out["events"], out["lost_power"] = verdicts, events, lost

    if lost:
        print("\nNot walking: the keyboard went away. Run again with --only walk once it is back.")
    elif args.only != "listen":
        if low:
            targets, level = [p for p in low if p not in exclude], 0
        elif only_pins:
            targets, level = [p for p in only_pins if p not in exclude], 1
        else:
            unsafe = {p for p, r in verdicts.items() if not r["kind"].startswith("quiet")}
            targets, level = [p for p in PINS if p not in exclude and p not in unsafe], 1
            if unsafe:
                print("\nNot driving: " + ", ".join(f"{p} ({verdicts[p]['kind']})" for p in PINS if p in unsafe))
        out["walk"] = {"level": level, "driven": walk(dev, targets, level, args.pulse_seconds)}
        print("\nDone. Tell Claude which F-key was lit when each of the four LEDs came on (or that none did).")

    path = pathlib.Path(args.out)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(out, indent=2), encoding="utf-8")
    print(f"Result saved to {path}")


if __name__ == "__main__":
    main()
