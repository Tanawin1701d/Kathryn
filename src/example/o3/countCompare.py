#!/usr/bin/env python3
"""
countCompare.py -- side-by-side 11-category comparison of the Kathryn Kride core
vs the reference RIDECORE Verilog, with per-category reduction %.

Reduction % per category = (RIDECORE - Kathryn) / RIDECORE * 100.

Also reports the derived rollups CTRL total (= sum of CTRL_* categories) and
DATA total (= sum of DATA_*), so the headline control-flow reduction is a
first-class output.

Paths are resolved relative to THIS script, so it runs from any CWD:
    Kathryn  = <o3>/core
    RIDECORE = <repo>/extSim/ridecore/src/fpga   (obj_dir auto-skipped)

Usage:
    python3 countCompare.py
"""
import os
import sys

from countMeasure import scan_folder, CATEGORIES, CTRL_CATS, DATA_CATS, fmt

HERE     = os.path.dirname(os.path.abspath(__file__))
KATHRYN  = os.path.join(HERE, "core")
RIDECORE = os.path.normpath(os.path.join(HERE, "..", "..", "..",
                                         "extSim", "ridecore", "src", "fpga"))


def totals(folder):
    per_cat, _, dc_excluded, _ = scan_folder(folder)
    return per_cat, dc_excluded


# per-category annotations shown in the RESULTS-style table
NOTES = {
    "PARAM": "   (Kathryn MORE: C++ DSL named params)",
    "HLH":   "   (Kathryn MORE: host scaffolding)",
}


def row(label, k, r, note=""):
    red = f"{(r - k) / r * 100:7.2f}%" if r else "    n/a"
    print(f"  {label:11s}{fmt(k):>7s}{fmt(r):>10s}{red:>11s}{note}")


def main():
    if not os.path.isdir(RIDECORE):
        print(f"WARNING: RIDECORE Verilog not found at {RIDECORE}", file=sys.stderr)
        print("         (submodule not populated? run: git submodule update --init)",
              file=sys.stderr)

    k_cat, k_excl = totals(KATHRYN)
    r_cat, r_excl = totals(RIDECORE) if os.path.isdir(RIDECORE) else ({}, 0)

    print(f"Kathryn  : {KATHRYN}")
    print(f"RIDECORE : {RIDECORE}")
    print()
    print(f"  {'CATEGORY':11s}{'KATHRYN':>7s}{'RIDECORE':>10s}{'REDUCTION':>11s}")
    kt = rt = 0.0
    for cat in CATEGORIES:
        k = k_cat.get(cat, 0)
        r = r_cat.get(cat, 0)
        kt += k
        rt += r
        row(cat, k, r, NOTES.get(cat, ""))
    print("  " + "-" * 40)
    # derived control-flow / data rollups
    kctrl = sum(k_cat.get(c, 0) for c in CTRL_CATS)
    rctrl = sum(r_cat.get(c, 0) for c in CTRL_CATS)
    kdata = sum(k_cat.get(c, 0) for c in DATA_CATS)
    rdata = sum(r_cat.get(c, 0) for c in DATA_CATS)
    row("CTRL total", kctrl, rctrl, "   <-- HEADLINE control-flow reduction")
    row("DATA total", kdata, rdata)
    print("  " + "-" * 40)
    row("TOTAL", kt, rt)
    print(f"  {'DC(excl)':11s}{k_excl:>7d}{r_excl:>10d}")
    print()
    if rctrl:
        print(f"CONTROL-FLOW reduction (headline): "
              f"({fmt(rctrl)} - {fmt(kctrl)}) / {fmt(rctrl)} = "
              f"{(rctrl - kctrl) / rctrl * 100:.2f}%")


if __name__ == "__main__":
    main()
