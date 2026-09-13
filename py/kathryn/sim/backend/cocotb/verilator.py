# Sim backend VERILATOR — the default simulator.
# - cocotb 2.x needs Verilator >= 5.036: put_verilator_on_path() puts the PyPI
#   wheel's bin/ (else a dedicated conda env) on PATH before cocotb looks.
# - The emitted Verilog writes `<=` inside always @(*) (COMBDLY), which
#   Verilator refuses by default, so -Wno-fatal is always passed;
#   --public-flat-rw keeps INTERNAL nets visible and forceable over VPI.

from __future__ import annotations

import os
import pathlib
import re
import shutil
import subprocess
from typing import List, Optional, Tuple

from ..base import read_version_line
from .base import CocotbBackend

VERILATOR_MIN   = (5, 36)                 # cocotb 2.x's floor; the wheel "5.48.0" is Verilator 5.048
VERILATOR_CONDA = pathlib.Path.home() / "miniconda3" / "envs" / "verilator" / "bin"


class VerilatorBackend(CocotbBackend):
    name = "verilator"

    def build_args(self) -> List[str]:          return ["-Wno-fatal", "--public-flat-rw"]
    def waves_file(self, toplevel: str) -> str: return "dump.vcd"       # cocotb's fixed name

    def describe(self) -> str:
        put_verilator_on_path()                  # the one that builds is the one described
        return read_version_line(shutil.which("verilator"))

    def make_runner(self):
        put_verilator_on_path()
        return make_verilator_runner()


def make_verilator_runner():
    """cocotb's Verilator runner, with the wheel's precompiled-header flag repaired.

    - the wheel's verilated.mk leaves CFG_CXXFLAGS_PCH_I empty, so a parallel
      build passes `Vtop__pch.h.fast` to c++ as a bare file and fails; giving
      make the flag on its command line overrides the makefile
    """
    from cocotb_tools.runner import Verilator      # local: cocotb is the optional `sim` extra

    class VerilatorWithPch(Verilator):
        def _build_command(self):
            cmds = super()._build_command()
            for cmd in cmds:
                if cmd and cmd[0] == "make":
                    cmd.append("CFG_CXXFLAGS_PCH_I=-include")
            return cmds

    return VerilatorWithPch()


# ---- finding a new-enough tool -----------------------------------------------

def parse_verilator_version(text: str) -> Optional[Tuple[int, int]]:
    """(major, minor) from a `verilator --version` line, or None."""
    found = re.match(r"Verilator\s+(\d+)\.(\d+)", text)
    return (int(found.group(1)), int(found.group(2))) if found else None


def read_verilator_version(exe: str) -> Optional[Tuple[int, int]]:
    try:
        out = subprocess.run([exe, "--version"], capture_output=True, text=True).stdout
    except OSError:
        return None
    return parse_verilator_version(out)


def is_new_enough(exe: Optional[str]) -> bool:
    return bool(exe) and (read_verilator_version(exe) or (0, 0)) >= VERILATOR_MIN


def find_wheel_verilator() -> Optional[pathlib.Path]:
    """The PyPI wheel's bin/ when it is installed and new enough, else None."""
    # The wheel's binary prints no version, so the wheel metadata is the check.
    try:
        import verilator as wheel
        from importlib.metadata import version
        major, minor = (int(part) for part in version("verilator").split(".")[:2])
        bin_dir      = pathlib.Path(wheel.__file__).parent / "bin"
    except Exception:                                  # noqa: BLE001 — absent or broken wheel
        return None
    if (major, minor) < VERILATOR_MIN or not (bin_dir / "verilator").is_file():
        return None
    return bin_dir


def find_conda_verilator() -> Optional[pathlib.Path]:
    """A dedicated conda env's bin/ when it holds a new-enough verilator, else None."""
    exe = VERILATOR_CONDA / "verilator"
    return VERILATOR_CONDA if exe.is_file() and is_new_enough(str(exe)) else None


def put_verilator_on_path() -> None:
    """Make sure the `verilator` cocotb finds on PATH is >= VERILATOR_MIN.

    - a new-enough PATH verilator wins; else the wheel, else the conda env
    """
    exe = shutil.which("verilator")
    if is_new_enough(exe):
        return
    bin_dir = find_wheel_verilator() or find_conda_verilator()
    if bin_dir is None:
        found = f"{exe} is too old" if exe else "none on PATH"
        raise RuntimeError(
            f"verilator >= {VERILATOR_MIN[0]}.{VERILATOR_MIN[1]:03d} is needed by cocotb 2 "
            f"({found}) — install the wheel with: pip install verilator")
    os.environ["PATH"] = os.pathsep.join([str(bin_dir), os.environ.get("PATH", "")])


def verilator_is_available() -> bool:
    """For test skip guards: a new-enough verilator on PATH, as the wheel, or in conda."""
    return (is_new_enough(shutil.which("verilator"))
            or find_wheel_verilator() is not None
            or find_conda_verilator() is not None)
