"""
Checks firmware (and optionally LittleFS data) size against budget thresholds.

Reads the RAM/Flash summary lines PlatformIO prints at the end of `pio run`, e.g.
    Flash: [========  ]  83.6% (used 1753621 bytes from 2097152 bytes)
and warns/fails when usage crosses the thresholds below. Prints GitHub Actions annotations
(::warning:: / ::error::) and appends a table to $GITHUB_STEP_SUMMARY when running in CI, so the
same script works locally:

    pio run -e esp32dev | tee build.log
    python scripts/check_size.py build.log --fs-dir data

Exit code is 1 if any budget is over its fail threshold, 0 otherwise (warnings don't fail).
"""

import argparse
import os
import re
import sys

# Percent thresholds: (warn at, fail at)
FLASH_THRESHOLDS = (85.0, 95.0)
RAM_THRESHOLDS = (70.0, 85.0)
FS_THRESHOLDS = (80.0, 95.0)

# LittleFS data partition size in bytes; keep in sync with the "datasize" script in package.json.
FS_BUDGET_DEFAULT = 1966080

SIZE_LINE_RE = re.compile(r"^(RAM|Flash):.*?\(used (\d+) bytes from (\d+) bytes\)", re.MULTILINE)

IN_CI = os.environ.get("GITHUB_ACTIONS") == "true"


def annotate(level, message):
    if IN_CI:
        print(f"::{level}::{message}")
    else:
        print(f"{level.upper()}: {message}")


def check(name, used, total, thresholds):
    """Return (status, percent) for one budget, printing an annotation if over a threshold."""
    warn_at, fail_at = thresholds
    pct = used / total * 100
    free_kb = (total - used) / 1024
    detail = f"{name} at {pct:.1f}% ({used:,} / {total:,} bytes, {free_kb:,.0f} KB free)"

    if pct >= fail_at:
        annotate("error", f"{detail} is over the {fail_at:.0f}% limit")
        return "❌ over limit", pct
    if pct >= warn_at:
        annotate("warning", f"{detail} is over the {warn_at:.0f}% warning level")
        return "⚠️ warning", pct

    print(f"OK: {detail}")
    return "✅ ok", pct


def dir_size(path):
    """Total size of every file under `path`, matching what `pio run -t uploadfs` sends (same as `du -sb`)."""
    total = 0
    for root, _, files in os.walk(path):
        for f in files:
            total += os.path.getsize(os.path.join(root, f))
    return total


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("build_log", help="Captured output of `pio run`")
    parser.add_argument("--fs-dir", help="Directory uploaded to LittleFS (e.g. data); skipped if omitted")
    parser.add_argument("--fs-budget", type=int, default=FS_BUDGET_DEFAULT, help="LittleFS partition size in bytes")
    args = parser.parse_args()

    with open(args.build_log, encoding="utf-8", errors="replace") as f:
        log = f.read()

    found = {m.group(1): (int(m.group(2)), int(m.group(3))) for m in SIZE_LINE_RE.finditer(log)}
    if "Flash" not in found or "RAM" not in found:
        annotate("error", f"Couldn't find RAM/Flash size lines in {args.build_log}; did the build succeed?")
        return 1

    rows = [
        ("Flash (firmware)", *found["Flash"], *check("Flash", *found["Flash"], FLASH_THRESHOLDS)),
        ("RAM (static)", *found["RAM"], *check("RAM", *found["RAM"], RAM_THRESHOLDS)),
    ]

    if args.fs_dir:
        if os.path.isdir(args.fs_dir):
            used = dir_size(args.fs_dir)
            rows.append(("LittleFS (data/)", used, args.fs_budget, *check("LittleFS", used, args.fs_budget, FS_THRESHOLDS)))
        else:
            annotate("warning", f"--fs-dir {args.fs_dir} doesn't exist; skipping the LittleFS check")

    summary_path = os.environ.get("GITHUB_STEP_SUMMARY")
    if summary_path:
        with open(summary_path, "a", encoding="utf-8") as f:
            f.write("### Size budget\n\n| Budget | Used | Total | % | Status |\n| --- | ---: | ---: | ---: | --- |\n")
            for name, used, total, status, pct in rows:
                f.write(f"| {name} | {used:,} | {total:,} | {pct:.1f}% | {status} |\n")

    return 1 if any(status.startswith("❌") for *_, status, _ in rows) else 0


if __name__ == "__main__":
    sys.exit(main())
