# Verilator backend + the tool discovery it needs.
#
# cocotb 2.x requires Verilator >= 5.036 (its verilator.cpp uses newer VPI APIs),
# so make_runner() resolves a new-enough binary onto PATH before handing back the
# stock cocotb runner. It also declares the extra compile flags Verilator needs.

from __future__ import annotations

import os
import re
import shutil
import subprocess
import pathlib

from .backend import SimBackend

# cocotb 2.x needs Verilator >= 5.036 (its verilator.cpp uses newer VPI APIs).
_VERILATOR_MIN   = (5, 36)
_VERILATOR_CONDA = pathlib.Path.home() / "miniconda3" / "envs" / "verilator" / "bin"


def _verilator_version(exe: str) -> tuple[int, int] | None:
    # `verilator --version` → "Verilator 5.050 2025-..."; None when unparsable.
    try:
        out = subprocess.run([exe, "--version"], capture_output=True, text=True).stdout
        m   = re.match(r"Verilator\s+(\d+)\.(\d+)", out)
        return (int(m.group(1)), int(m.group(2))) if m else None
    except OSError:
        return None


def _wheel_verilator_bin() -> pathlib.Path | None:
    # The PyPI `verilator` wheel bundles the full install tree (perl driver,
    # verilator_bin, headers) inside the package. Its binary prints no version
    # number (`rev vUNKNOWN-built...`), so validate via the wheel metadata
    # instead (e.g. "5.48.0" == Verilator 5.048).
    try:
        import verilator as _vl_pkg
        from importlib.metadata import version
        ver     = tuple(int(x) for x in version("verilator").split(".")[:2])
        bin_dir = pathlib.Path(_vl_pkg.__file__).parent / "bin"
    except Exception:                     # noqa: BLE001 — wheel absent/broken
        return None
    return bin_dir if ver >= _VERILATOR_MIN and (bin_dir / "verilator").is_file() else None


def _ensure_verilator() -> None:
    # A PATH verilator that is new enough wins; otherwise fall back to the PyPI
    # wheel (`pip install verilator`), then a dedicated conda env, by prepending
    # the chosen bin dir to PATH (cocotb resolves the tool via PATH).
    exe = shutil.which("verilator")
    if exe and (_verilator_version(exe) or (0, 0)) >= _VERILATOR_MIN:
        return
    bin_dir = _wheel_verilator_bin()
    if bin_dir is None:
        conda_exe = _VERILATOR_CONDA / "verilator"
        if conda_exe.is_file() and (_verilator_version(str(conda_exe)) or (0, 0)) >= _VERILATOR_MIN:
            bin_dir = _VERILATOR_CONDA
    if bin_dir is not None:
        os.environ["PATH"] = os.pathsep.join([str(bin_dir), os.environ.get("PATH", "")])
        return
    found = f"{exe} is too old" if exe else "none found in PATH"
    raise RuntimeError(
        f"verilator >= {_VERILATOR_MIN[0]}.{_VERILATOR_MIN[1]:03d} required by cocotb 2.x ({found}); "
        "install one with: pip install verilator"
    )


class VerilatorBackend(SimBackend):
    def __init__(self):
        super().__init__("verilator")

    def build_args(self) -> list[str]:
        # Verilator treats lint warnings as fatal by default; the emitted Verilog
        # uses non-blocking `<=` in `always @(*)` blocks (COMBDLY), so keep
        # warnings non-fatal — matching iverilog's tolerance.
        return ["-Wno-fatal"]

    def waves_file(self, toplevel: str) -> str:
        # cocotb's Verilator runner always dumps to a fixed "dump.vcd".
        return "dump.vcd"

    def make_runner(self):
        _ensure_verilator()
        from cocotb_tools.runner import get_runner
        return get_runner("verilator")
