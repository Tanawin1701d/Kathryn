#!/usr/bin/env python3
# Build + run riscv-tests suites on the Kathryn core via the simharness.
#
#   .venv/bin/python tools/riscv_tests/run.py [--suite rv64ui] [--preset rv64i_min] [name ...]
#
# Exit convention (env/riscv_test.h): tohost=1 pass; odd n = failing TESTNUM n>>1.

from __future__ import annotations

import argparse
import pathlib
import subprocess
import sys
import time

HERE  = pathlib.Path(__file__).resolve().parent
REPO  = HERE.parents[1]
TESTS = HERE / "riscv-tests"
OUT   = HERE / ".out"

CROSS   = "riscv64-unknown-elf-gcc"
OBJCOPY = "riscv64-unknown-elf-objcopy"

RAM_BASE   = 0x8000_0000
TOHOST     = 0x8020_0000
MAX_CYCLES = 5_000_000

MARCH = {"rv64ui": "rv64i", "rv64um": "rv64im", "rv64ua": "rv64ia", "rv64mi": "rv64i"}


def sh(cmd, **kw):
    r = subprocess.run([str(c) for c in cmd], capture_output=True, text=True, **kw)
    if r.returncode != 0:
        raise RuntimeError(f"{' '.join(str(c) for c in cmd)}\n{r.stdout}\n{r.stderr}")
    return r


def build_core_and_harness(preset: str) -> pathlib.Path:
    rtl = OUT / f"rtl_{preset}"
    sh([sys.executable, "-m", "riscv.gen", "--preset", preset, "--out", rtl],
       cwd=REPO, env={"PYTHONPATH": str(REPO / "py"), "PATH": "/usr/bin:/bin"})
    sh([sys.executable, REPO / "tools/simharness/build.py", "--verilog-dir", rtl])
    return rtl / "sim" / "obj_dir" / "simharness"


def build_test(src: pathlib.Path, march: str) -> pathlib.Path | None:
    elf, binf = OUT / f"{src.stem}.elf", OUT / f"{src.stem}.bin"
    try:
        sh([CROSS, f"-march={march}", "-mabi=lp64", "-static", "-nostdlib",
            "-nostartfiles", "-mcmodel=medany",
            f"-I{HERE / 'env'}", f"-I{TESTS / 'isa' / 'macros' / 'scalar'}",
            "-T", HERE / "env" / "link.ld", "-o", elf, src])
        sh([OBJCOPY, "-O", "binary", elf, binf])
    except RuntimeError as e:
        print(f"COMPILE-ERR {src.stem}: {e}", file=sys.stderr)
        return None
    return binf


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--suite",  default="rv64ui")
    ap.add_argument("--preset", default="rv64i_min")
    ap.add_argument("names", nargs="*")
    args = ap.parse_args()

    if not TESTS.is_dir():
        sys.exit("run.py: riscv-tests checkout missing — run tools/riscv_tests/fetch.sh")
    OUT.mkdir(exist_ok=True)
    exe = build_core_and_harness(args.preset)

    srcs = sorted((TESTS / "isa" / args.suite).glob("*.S"))
    if args.names:
        srcs = [s for s in srcs if s.stem in args.names]
    if not srcs:
        sys.exit("run.py: no tests matched")

    march  = MARCH.get(args.suite, "rv64i")
    failed = []
    t0     = time.monotonic()
    total_cycles = 0
    for src in srcs:
        binf = build_test(src, march)
        if binf is None:
            failed.append(src.stem)
            continue
        r = subprocess.run(
            [str(exe), f"+image={binf}@{RAM_BASE:x}", f"+tohost=0x{TOHOST:x}",
             f"+max-cycles={MAX_CYCLES}"],
            capture_output=True, text=True, stdin=subprocess.DEVNULL)
        cyc = 0
        for line in r.stderr.splitlines():
            if "done rc=" in line:
                cyc = int(line.rsplit(" ", 2)[-2])
        total_cycles += cyc
        if r.returncode == 0:
            print(f"PASS       {src.stem}")
        else:
            tag = "TIMEOUT" if r.returncode == 124 else f"FAIL(test {r.returncode})"
            print(f"{tag:10s} {src.stem}")
            failed.append(src.stem)
    dt = time.monotonic() - t0
    rate = total_cycles / dt / 1e6 if dt else 0
    print(f"\n{len(srcs) - len(failed)}/{len(srcs)} passed   "
          f"({total_cycles} cycles, {dt:.1f}s wall, ~{rate:.2f} Mcyc/s)")
    if failed:
        print("failed:", " ".join(failed))
    sys.exit(1 if failed else 0)


if __name__ == "__main__":
    main()
