#!/usr/bin/env python3
# Verilate an emitted-Verilog directory against the simharness C++ models.
#
#   python tools/simharness/build.py --verilog-dir <dir> [--top NAME] [--trace]
#       → <out>/obj_dir/simharness
#
# The model is always built with `--prefix Vdut`, so the C++ side never needs
# the (mangled) Kathryn top-module name.

from __future__ import annotations

import argparse
import os
import pathlib
import re
import subprocess
import sys

HERE = pathlib.Path(__file__).resolve().parent
REPO = HERE.parents[1]


def find_verilator() -> str:
    # Prefer the shared resolver in test/cocotb_pool (PATH → PyPI wheel → conda);
    # fall back to a local wheel probe when cocotb_pool's deps aren't importable.
    try:
        sys.path.insert(0, str(REPO / "test"))
        from cocotb_pool.verilator import ensure_verilator
        return ensure_verilator()
    except Exception:
        pass
    try:
        import verilator as _vl_pkg
        bin_dir = pathlib.Path(_vl_pkg.__file__).parent / "bin"
        exe     = bin_dir / "verilator"
        if exe.is_file():
            os.environ["PATH"] = os.pathsep.join([str(bin_dir), os.environ.get("PATH", "")])
            return str(exe)
    except Exception:
        pass
    raise SystemExit("simharness: no verilator >= 5.036 found (pip install verilator)")


def toplevel_from_verilog(top_v: pathlib.Path) -> str:
    m = re.search(r"^\s*module\s+([A-Za-z_][A-Za-z0-9_$]*)", top_v.read_text(), re.M)
    if not m:
        raise SystemExit(f"simharness: no module declaration found in {top_v}")
    return m.group(1)


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--verilog-dir", required=True, help="directory of emitted .v files")
    ap.add_argument("--top",   default=None, help="top module name (default: parse top.v)")
    ap.add_argument("--out",   default=None, help="build dir (default: <verilog-dir>/sim)")
    ap.add_argument("--trace", action="store_true", help="compile with VCD tracing support")
    ap.add_argument("-j", "--jobs", type=int, default=os.cpu_count() or 4)
    args = ap.parse_args()

    vdir = pathlib.Path(args.verilog_dir).resolve()
    out  = pathlib.Path(args.out).resolve() if args.out else vdir / "sim"
    out.mkdir(parents=True, exist_ok=True)

    sources = sorted(str(p) for p in vdir.glob("*.v"))
    if not sources:
        raise SystemExit(f"simharness: no .v files in {vdir}")
    top = args.top or toplevel_from_verilog(vdir / "top.v")

    verilator = find_verilator()
    cmd = [
        verilator, "--cc", "--exe", "--build", "-j", str(args.jobs),
        "-O3", "-Wno-fatal",
        "--top-module", top, "--prefix", "Vdut",
        "--Mdir", str(out / "obj_dir"),
        "-o", "simharness",
        "-CFLAGS", "-O2 -std=c++17",
        *(["--trace"] if args.trace else []),
        *sources,
        str(HERE / "cpp" / "sim_main.cpp"),
        str(HERE / "cpp" / "devices.cpp"),
    ]
    print("simharness:", " ".join(cmd), file=sys.stderr)
    subprocess.run(cmd, check=True)
    print(out / "obj_dir" / "simharness")


if __name__ == "__main__":
    main()
