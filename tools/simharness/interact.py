#!/usr/bin/env python3
# Expect-style console driver for the simharness: wait for output patterns,
# send responses, capture everything. Steps come as PATTERN=RESPONSE args:
#
#   interact.py --timeout 900 -- <sim cmd...> 'login:=root' '\\$|#=coremark' '#=exit'
#
# Each step waits (streaming) until PATTERN appears in new output, then sends
# RESPONSE + newline. After the last step, waits for --settle seconds of quiet
# (or --final-pattern) and exits 0. Exits 3 on overall timeout.

from __future__ import annotations

import argparse
import os
import re
import select
import subprocess
import sys
import time


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--timeout", type=float, default=900)
    ap.add_argument("--settle",  type=float, default=8, help="quiet seconds after last step")
    ap.add_argument("--log",     default=None)
    ap.add_argument("rest", nargs=argparse.REMAINDER)
    args = ap.parse_args()

    rest = args.rest[1:] if args.rest and args.rest[0] == "--" else args.rest
    cmd, steps = [], []
    for a in rest:
        (steps if ("=" in a and not a.startswith(("+", "-", "/")) and not os.path.exists(a.split("=")[0])) else cmd).append(a)
    steps = [s.split("=", 1) for s in steps]

    proc = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, bufsize=0)
    logf = open(args.log, "wb") if args.log else None
    buf, transcript = b"", b""
    deadline  = time.monotonic() + args.timeout
    last_out  = time.monotonic()
    step_i    = 0

    try:
        while time.monotonic() < deadline:
            r, _, _ = select.select([proc.stdout], [], [], 0.5)
            if r:
                chunk = proc.stdout.read(4096)
                if not chunk:
                    break
                buf        += chunk
                transcript += chunk
                last_out    = time.monotonic()
                sys.stdout.buffer.write(chunk); sys.stdout.flush()
                if logf: logf.write(chunk); logf.flush()
            if step_i < len(steps):
                pat, resp = steps[step_i]
                if re.search(pat.encode(), buf):
                    proc.stdin.write(resp.encode() + b"\n"); proc.stdin.flush()
                    buf     = b""
                    step_i += 1
            elif time.monotonic() - last_out > args.settle:
                proc.kill()
                sys.exit(0)
        proc.kill()
        sys.exit(0 if step_i == len(steps) else 3)
    finally:
        if logf: logf.close()


if __name__ == "__main__":
    main()
