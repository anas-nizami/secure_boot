#!/usr/bin/env python3
"""
Checks that tools/sign_image.py's hand-written header layout still matches
bootloader/inc/img_header.h.

sign_image.py builds the 512-byte header with
    struct.pack('<IIII32s64s', MAGIC, version, len(body), 0, digest, signature)
which hardcodes the field order, offsets and sizes of img_header_t. Nothing
in the C build enforces that this stays in sync with img_header.h, so a
field reorder/resize on the C side would silently desync the signer from the
bootloader. This script asks the compiler what it actually laid out (via
dump_format.c in this directory, built from img_header.h with
offsetof/sizeof) and compares it against what sign_image.py assumes.

Run via `make run_format_sync` (builds dump_format.exe first) or
directly once that binary has been built.
"""
import subprocess
import sys
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
REPO_ROOT = SCRIPT_DIR.parent.parent
DUMP_BIN = SCRIPT_DIR / 'dump_format.exe'

sys.path.insert(0, str(REPO_ROOT / 'tools'))
import sign_image  # noqa: E402  (path must be set up first) - reuse the real constants, don't retype them

# Offsets/sizes implied by sign_image.py's struct.pack('<IIII32s64s', ...):
# 4 x uint32 (magic, version, img_len, reserved), then a 32-byte hash, then a 64-byte sig.
EXPECTED_OFFSETS = {
    'OFF_MAGIC': 0,
    'OFF_VERSION': 4,
    'OFF_IMG_LEN': 8,
    'OFF_RESERVED': 12,
    'OFF_HASH': 16,
    'OFF_SIG': 48,
}
EXPECTED_SIZES = {
    'SIZE_HASH': 32,
    'SIZE_SIG': 64,
}


def get_c_format():
    if not DUMP_BIN.exists():
        print(f"Error: {DUMP_BIN} not found - build it first (make {DUMP_BIN.name} or make run_format_sync)")
        sys.exit(1)
    try:
        output = subprocess.check_output([str(DUMP_BIN)], text=True)
    except (subprocess.CalledProcessError, OSError) as e:
        print(f"Error: failed to run {DUMP_BIN}: {e}")
        sys.exit(1)

    print(f"--- {DUMP_BIN.name} output ---")
    print(output, end='' if output.endswith('\n') else '\n')
    print("---")

    fields = {}
    for line in output.strip().splitlines():
        key, _, value = line.partition('=')
        fields[key] = value
    return fields


def check():
    c = get_c_format()
    failures = []

    if int(c['MAGIC'], 0) != sign_image.MAGIC:
        failures.append(f"MAGIC mismatch: img_header.h=0x{int(c['MAGIC'], 0):08x} sign_image.py=0x{sign_image.MAGIC:08x}")

    if int(c['HEADER_SIZE']) != sign_image.HEADER_SIZE:
        failures.append(f"HEADER_SIZE mismatch: img_header.h={c['HEADER_SIZE']} sign_image.py={sign_image.HEADER_SIZE}")

    for key, expected in EXPECTED_OFFSETS.items():
        actual = int(c[key])
        if actual != expected:
            failures.append(f"{key} mismatch: img_header.h={actual} sign_image.py assumes={expected}")

    for key, expected in EXPECTED_SIZES.items():
        actual = int(c[key])
        if actual != expected:
            failures.append(f"{key} mismatch: img_header.h={actual} sign_image.py assumes={expected}")

    if failures:
        print("FAIL: sign_image.py's header layout no longer matches img_header_t:")
        for f in failures:
            print(f"  - {f}")
        print("Update the struct.pack() format string (and MAGIC/HEADER_SIZE) in tools/sign_image.py to match.")
        sys.exit(1)

    print("OK: sign_image.py header layout matches img_header_t")


if __name__ == '__main__':
    check()
