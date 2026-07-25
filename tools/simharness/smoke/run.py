#!/usr/bin/env python3
# Smoke test for the simharness: build the hand-written smoke.v DUT, run it,
# and assert the UART printed "Hi" and the HTIF reported pass (exit 0).

from __future__ import annotations

import pathlib
import subprocess
import sys

HERE = pathlib.Path(__file__).resolve().parent


def main() -> None:
    out = HERE / ".out"
    out.mkdir(exist_ok=True)

    build = subprocess.run(
        [sys.executable, str(HERE.parent / "build.py"),
         "--verilog-dir", str(HERE), "--top", "smoke", "--out", str(out)],
        capture_output=True, text=True)
    if build.returncode != 0:
        sys.exit(f"smoke: harness build failed\n{build.stdout}\n{build.stderr}")
    exe = build.stdout.strip().splitlines()[-1]

    run = subprocess.run(
        [exe, "+tohost=0x80001000", "+max-cycles=10000", "+quiet"],
        capture_output=True, text=True, stdin=subprocess.DEVNULL)
    assert run.returncode == 0, f"smoke: sim rc={run.returncode}\n{run.stdout}\n{run.stderr}"
    assert "Hi" in run.stdout,  f"smoke: UART output missing 'Hi': {run.stdout!r}"
    print("simharness smoke: PASS")


if __name__ == "__main__":
    main()
