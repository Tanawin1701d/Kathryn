# Verilog generation entry point:
#   PYTHONPATH=py python -m riscv.gen --preset rv64i_min --out build/rtl

from __future__ import annotations

import argparse
import pathlib
import sys

REPO = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(REPO / "py"))

from kathryn import reset, build_model, emit_verilog          # noqa: E402

from .config import PRESETS                                   # noqa: E402
from .core   import RV64Core                                  # noqa: E402


def generate(preset: str, out_dir: str) -> pathlib.Path:
    soc_cfg = PRESETS[preset]
    out     = pathlib.Path(out_dir)
    out.mkdir(parents=True, exist_ok=True)
    reset()
    build_model(RV64Core(soc_cfg.core, "core"))
    emit_verilog(str(out))
    return out


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--preset", default="rv64i_min", choices=sorted(PRESETS))
    ap.add_argument("--out",    required=True)
    args = ap.parse_args()
    out = generate(args.preset, args.out)
    print(out)


if __name__ == "__main__":
    main()
