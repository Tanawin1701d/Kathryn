#!/usr/bin/env python3
# Micro-test runner: generate the core once, build the simharness once, then
# assemble + run every test/riscv/sw/tests/*.S through it (tohost pass/fail).
#
#   .venv/bin/python test/riscv/run_micro.py [name ...]

from __future__ import annotations

import pathlib
import subprocess
import sys

HERE = pathlib.Path(__file__).resolve().parent
REPO = HERE.parents[1]
OUT  = HERE / ".out"
SW   = HERE / "sw"

CROSS   = "riscv64-unknown-elf-gcc"
OBJCOPY = "riscv64-unknown-elf-objcopy"
MARCH   = "rv64i"

RAM_BASE   = 0x8000_0000
TOHOST     = 0x8000_1000
MAX_CYCLES = 2_000_000


def sh(cmd, **kw):
    r = subprocess.run([str(c) for c in cmd], capture_output=True, text=True, **kw)
    if r.returncode != 0:
        sys.exit(f"FAILED: {' '.join(str(c) for c in cmd)}\n{r.stdout}\n{r.stderr}")
    return r


def build_core_and_harness() -> pathlib.Path:
    # zicsr preset: a superset of rv64i_min — all micro tests run on it.
    rtl = OUT / "rtl"
    sh([sys.executable, "-m", "riscv.gen", "--preset", "rv64i_zicsr", "--out", rtl],
       cwd=REPO, env={"PYTHONPATH": str(REPO / "py"), "PATH": "/usr/bin:/bin"})
    sh([sys.executable, REPO / "tools/simharness/build.py", "--verilog-dir", rtl])
    return rtl / "sim" / "obj_dir" / "simharness"


def build_prog(src: pathlib.Path) -> pathlib.Path:
    elf, binf = OUT / f"{src.stem}.elf", OUT / f"{src.stem}.bin"
    sh([CROSS, f"-march={MARCH}", "-mabi=lp64", "-nostdlib", "-nostartfiles",
        "-static", f"-I{SW}", "-T", SW / "link.ld", "-o", elf, src])
    sh([OBJCOPY, "-O", "binary", elf, binf])
    return binf


def run_one(exe: pathlib.Path, binf: pathlib.Path) -> tuple[int, str]:
    r = subprocess.run(
        [str(exe), f"+image={binf}@{RAM_BASE:x}", f"+tohost=0x{TOHOST:x}",
         f"+max-cycles={MAX_CYCLES}", "+quiet"],
        capture_output=True, text=True, stdin=subprocess.DEVNULL)
    return r.returncode, r.stdout


def main() -> None:
    only = set(sys.argv[1:])
    OUT.mkdir(exist_ok=True)
    exe = build_core_and_harness()

    tests = sorted((SW / "tests").glob("*.S"))
    if only:
        tests = [t for t in tests if t.stem in only]
    if not tests:
        sys.exit("run_micro: no tests matched")

    failed = []
    for src in tests:
        rc, out = run_one(exe, build_prog(src))
        status = "PASS" if rc == 0 else ("TIMEOUT" if rc == 124 else f"FAIL({rc})")
        print(f"{status:10s} {src.stem}")
        if rc != 0:
            failed.append(src.stem)
    print(f"\n{len(tests) - len(failed)}/{len(tests)} passed")
    sys.exit(1 if failed else 0)


if __name__ == "__main__":
    main()
