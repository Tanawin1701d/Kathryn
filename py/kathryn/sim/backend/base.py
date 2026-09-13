# Sim backend BASE — what any simulator backend is, whichever harness drives it.
# - A backend names its tool and what a build under it depends on — the cache
#   key reads exactly these: name, build_args, describe — and compiles an Rtl
#   into a build dir.  HOW it compiles, and how a test is then run, is the
#   harness's business: see backend/cocotb/base.py.
# - NOT here: any harness vocabulary.  A harness is a sub-package extending
#   this class.

from __future__ import annotations

import pathlib
import subprocess
from abc import ABC, abstractmethod
from typing import List, Optional

from ..rtl import Rtl


class SimBackend(ABC):
    """One simulator: its tool, and what a build under it depends on."""

    name : str

    @abstractmethod
    def build_args(self) -> List[str]: ...          # extra flags a build under this tool needs
    @abstractmethod
    def waves_file(self, toplevel: str) -> str: ... # the dump a compiled sim writes into build_dir
    @abstractmethod
    def describe(self) -> str: ...                  # the tool's version line, part of the cache key

    @abstractmethod
    def compile(
        self,
        rtl       : Rtl,
        build_dir : pathlib.Path,
        waves     : bool = False,
        log_path  : Optional[pathlib.Path] = None,
    ) -> None: ...                                  # turn the emit into a simulator build in build_dir


def read_version_line(exe: Optional[str]) -> str:
    """"<exe>: <first line of --version>", or "missing" with no tool at all."""
    if exe is None:
        return "missing"
    try:
        out = subprocess.run([exe, "--version"], capture_output=True, text=True).stdout
    except OSError:
        return exe
    return f"{exe}: {out.strip().splitlines()[0] if out.strip() else '?'}"
