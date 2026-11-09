#!/usr/bin/env python3
"""Log readings from a RuiDeng UM34C USB power meter over Bluetooth, for the power tests.

The PWR-xx rows in test/README.md use it. Pair the meter and bind it to a serial port first
(e.g. /dev/rfcomm0), then:

    python3 scripts/um34c_log.py                       # until Ctrl-C, one sample a second
    python3 scripts/um34c_log.py --minutes 30 --csv tmp/pwr-01.csv

Each sample is printed. At the end you get the average, minimum and maximum current and power
over the run. The average is over evenly spaced samples, so the short spikes from WiFi and
display refreshes count for what they are worth.

Protocol: send the byte 0xF0 and the meter answers with a 130-byte, big-endian status packet.
Only the fields used here are decoded.
"""

import argparse
import csv
import signal
import struct
import sys
import time
from datetime import datetime

try:
    import serial
except ImportError:
    sys.exit("pyserial is missing: pip install pyserial")

PACKET_SIZE = 130
UM34C_MODEL = 0x0D4C


def read_sample(port):
    """Ask the meter for one status packet and decode it, or return None if it didn't answer."""
    port.reset_input_buffer()
    port.write(b"\xf0")
    data = port.read(PACKET_SIZE)
    if len(data) != PACKET_SIZE:
        return None

    def u16(offset):
        return struct.unpack(">H", data[offset : offset + 2])[0]

    def u32(offset):
        return struct.unpack(">I", data[offset : offset + 4])[0]

    if u16(0) != UM34C_MODEL:
        return None
    group = u16(14)
    return {
        "volts": u16(2) / 100,
        "amps": u16(4) / 1000,
        "watts": u32(6) / 1000,
        "celsius": u16(10),
        "group_mah": u32(16 + group * 8),  # the meter's own counter for the selected group
        "group_mwh": u32(20 + group * 8),
    }


def stop_on_sigterm(signum, frame):
    """Stopping with `kill` (SIGTERM) ends the run like Ctrl-C does: with the summary."""
    raise KeyboardInterrupt


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("--port", default="/dev/rfcomm0", help="serial port bound to the meter (default: %(default)s)")
    parser.add_argument("--interval", type=float, default=1.0, help="seconds between samples (default: %(default)s)")
    parser.add_argument("--minutes", type=float, help="stop after this long (default: until Ctrl-C)")
    parser.add_argument("--csv", help="also write every sample to this CSV file")
    args = parser.parse_args()

    signal.signal(signal.SIGTERM, stop_on_sigterm)

    port = serial.Serial(args.port, 9600, timeout=3)
    time.sleep(1)  # the Bluetooth link needs a moment after opening

    writer = None
    csv_file = None
    if args.csv:
        csv_file = open(args.csv, "w", newline="")
        writer = csv.writer(csv_file)
        writer.writerow(["time", "volts", "amps", "watts", "celsius", "group_mah", "group_mwh"])

    samples = []
    started = time.monotonic()
    end = started + args.minutes * 60 if args.minutes else None
    try:
        while end is None or time.monotonic() < end:
            tick = time.monotonic()
            sample = read_sample(port)
            now = datetime.now().isoformat(timespec="seconds")
            if sample is None:
                print(f"{now}  no answer from the meter", file=sys.stderr)
            else:
                samples.append(sample)
                print(f"{now}  {sample['volts']:.2f} V  {sample['amps'] * 1000:4.0f} mA  {sample['watts']:.3f} W")
                if writer:
                    writer.writerow([now] + [sample[key] for key in
                                             ("volts", "amps", "watts", "celsius", "group_mah", "group_mwh")])
                    csv_file.flush()
            time.sleep(max(0.0, args.interval - (time.monotonic() - tick)))
    except KeyboardInterrupt:
        pass
    finally:
        if csv_file:
            csv_file.close()

    if not samples:
        sys.exit("No samples.")
    minutes = (time.monotonic() - started) / 60
    amps = [s["amps"] for s in samples]
    watts = [s["watts"] for s in samples]
    print(f"\n{len(samples)} samples over {minutes:.1f} min at {samples[-1]['volts']:.2f} V")
    print(f"current: average {1000 * sum(amps) / len(amps):.1f} mA, min {1000 * min(amps):.0f}, max {1000 * max(amps):.0f}")
    print(f"power:   average {sum(watts) / len(watts):.3f} W, min {min(watts):.3f}, max {max(watts):.3f}")
    used = samples[-1]["group_mah"] - samples[0]["group_mah"]
    print(f"meter's own counter: {used} mAh over the run")


if __name__ == "__main__":
    main()
