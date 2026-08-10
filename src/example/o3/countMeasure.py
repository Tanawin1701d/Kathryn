#!/usr/bin/env python3
"""
countMeasure.py -- reproducible 11-category code-productivity counter.

Classifies marked lines into the categories defined in the sibling file
`count_measure`.  The scheme is a cross-product of a DOMAIN axis (control vs
data, decided by the keyword list in `count_measure`) and a KIND axis, plus
three standalone kinds:

    CTRL_HWD  CTRL_CL  CTRL_DT  CTRL_HC      (control domain x 4 kinds)
    DATA_HWD  DATA_CL  DATA_DT  DATA_HC      (data    domain x 4 kinds)
    MD        PARAM    HLH                   (standalone, no domain)

Marker syntax is a trailing comment:  <code>   ///<CAT> <GROUP>
ONE PHYSICAL LINE = ONE CATEGORY, score 1.0.  Multi-aspect statements are
line-broken so each physical line is one clean category.  The only inseparable
case is a single port bundle carrying both a control and a data signal, tagged
///CTRL_HC+DATA_HC <GROUP>, which credits 1/K score to EACH of its K categories
-- never 1.0 to each -- so a line always contributes 1.0 in total and
per-category totals can be fractional.  Lines carrying ///DC are EXCLUDED
(audit trail only).

Usage:
    python3 countMeasure.py [TARGET_DIR] [--json OUT.json]

    TARGET_DIR defaults to ./core (the Kathryn Kride core).
    For the RIDECORE Verilog, pass its fpga path, e.g.:
        python3 countMeasure.py ../../../extSim/ridecore/src/fpga
"""
import os
import re
import sys
import json
from collections import defaultdict

# the 11 categories.  There is no priority order; this list only fixes the
# report row order and the set of names the tag parser recognizes.
CATEGORIES = [
    "CTRL_HWD", "CTRL_CL", "CTRL_DT", "CTRL_HC",
    "DATA_HWD", "DATA_CL", "DATA_DT", "DATA_HC",
    "MD", "PARAM", "HLH",
]

# derived rollups reported by countCompare.py
CTRL_CATS = ["CTRL_HWD", "CTRL_CL", "CTRL_DT", "CTRL_HC"]
DATA_CATS = ["DATA_HWD", "DATA_CL", "DATA_DT", "DATA_HC"]

# A tag is  ///<CAT>[+<CAT2>...] <GROUP> .  Alternation is longest-first so no
# name can be shadowed by a prefix of another (none are prefixes here, but keep
# it robust).  The whole category expression (with any '+' parts) is group 1.
_ALT = "|".join(sorted(CATEGORIES, key=len, reverse=True))
# NB: wrap the alternation in (?:...) so the '+'-repeat binds to the whole
# category token, not just the last alternative.
# `(?<!/)///(?!/)` = EXACTLY a triple-slash tag, so 4+-slash comment runs
# (//// , ///// ) are never mistaken for a tag at any offset.
CAT_PATTERN = re.compile(rf"(?<!/)///(?!/)\s*((?:{_ALT})(?:\+(?:{_ALT}))*)\s+(\w+)")
DC_PATTERN  = re.compile(r"(?<!/)///(?!/)\s*DC\b")

# uppercase, tag-shaped token after ///, to catch stray/old category tags
# (e.g. leftover CTRL, HWD, HC_CTRL) without flagging ordinary /// prose comments.
ANY_TAG = re.compile(r"(?<!/)///(?!/)\s*([A-Z][A-Z0-9_]+(?:\+[A-Z0-9_]+)*)")
VALID_TAG = re.compile(rf"^(?:{_ALT})(?:\+(?:{_ALT}))*$")


def fmt(x):
    """Print an int as an int, a fractional total with one decimal."""
    return str(int(x)) if float(x).is_integer() else f"{x:.1f}"

# generated / build dirs that must never be scanned (would double-count)
SKIP_DIRS = {"obj_dir", ".git", "build", "generated", "deleted"}


def scan_folder(folder):
    """Return (per_cat, per_cat_group, dc_excluded, unknown_tags)."""
    per_cat       = defaultdict(float)
    per_cat_group = defaultdict(float)   # key = (cat, group)
    dc_excluded   = 0
    unknown_tags  = defaultdict(int)     # stray ///<tag> that matched no category
    for root, dirs, files in os.walk(folder):
        dirs[:] = [d for d in dirs
                   if d not in SKIP_DIRS and not d.startswith("cmake-build")]
        for fname in files:
            path = os.path.join(root, fname)
            try:
                with open(path, "r", encoding="utf-8", errors="ignore") as f:
                    for line in f:
                        # ///DC lines are excluded outright
                        if DC_PATTERN.search(line):
                            dc_excluded += 1
                            continue
                        matches = CAT_PATTERN.findall(line)
                        if not matches:
                            # flag any ///tag that isn't a recognized category
                            if "///" in line:
                                for tok in ANY_TAG.findall(line):
                                    if tok != "DC" and not VALID_TAG.match(tok):
                                        unknown_tags[tok] += 1
                            continue
                        for cat_expr, group in matches:
                            cats = [c for c in cat_expr.split("+") if c]
                            share = 1.0 / len(cats)
                            for cat in cats:
                                per_cat[cat]              += share
                                per_cat_group[(cat, group)] += share
            except Exception as e:
                print(f"Skip {path}: {e}", file=sys.stderr)
    return per_cat, per_cat_group, dc_excluded, unknown_tags


def report(folder):
    per_cat, per_cat_group, dc_excluded, unknown = scan_folder(folder)
    total = sum(per_cat.values())
    print(f"=== 11-CATEGORY COUNTS : {folder} ===")
    print(f"{'CATEGORY':9s}  {'COUNT':>6s}")
    for cat in CATEGORIES:
        print(f"{cat:9s}  {fmt(per_cat.get(cat, 0)):>6s}")
    print("-" * 18)
    ctrl_t = sum(per_cat.get(c, 0) for c in CTRL_CATS)
    data_t = sum(per_cat.get(c, 0) for c in DATA_CATS)
    print(f"{'CTRL tot':9s}  {fmt(ctrl_t):>6s}")
    print(f"{'DATA tot':9s}  {fmt(data_t):>6s}")
    print(f"{'TOTAL':9s}  {fmt(total):>6s}")
    print(f"{'DC(excl)':9s}  {dc_excluded:6d}")
    bad = sorted({g for (c, g) in per_cat_group if g in CATEGORIES})
    if bad:
        print(f"WARNING: group name(s) collide with a CATEGORY name (fix these): {', '.join(bad)}")
    if unknown:
        tot = sum(unknown.values())
        print(f"WARNING: {tot} stray ///tag(s) matched no category (fix these): "
              + ", ".join(f"{t}:{n}" for t, n in sorted(unknown.items())))
    print()
    print("--- per category x group ---")
    for cat in CATEGORIES:
        groups = sorted((g, n) for (c, g), n in per_cat_group.items() if c == cat)
        if groups:
            print(f"{cat:9s}  " + ", ".join(f"{g}:{fmt(n)}" for g, n in groups))
    return {
        "folder": folder,
        "per_category": {c: per_cat.get(c, 0) for c in CATEGORIES},
        "ctrl_total": ctrl_t,
        "data_total": data_t,
        "per_category_group": {f"{c}/{g}": n for (c, g), n in per_cat_group.items()},
        "total": total,
        "dc_excluded": dc_excluded,
        "unknown_tags": dict(unknown),
    }


if __name__ == "__main__":
    pos = [a for a in sys.argv[1:] if not a.startswith("--")]
    target = pos[0] if pos else "./core"
    summary = report(target)
    if "--json" in sys.argv:
        out = sys.argv[sys.argv.index("--json") + 1]
        with open(out, "w") as f:
            json.dump(summary, f, indent=2)
        print(f"\n(wrote {out})")
