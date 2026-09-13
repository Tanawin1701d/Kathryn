# backend — which simulator runs an emitted design.
# - base.py holds what any backend is; each HARNESS is a sub-package that
#   extends it and holds one file per tool it drives.
#
#   base.py     SimBackend: name, build_args, waves_file, describe, compile
#   cocotb/     the cocotb harness: CocotbBackend, then Verilator and Icarus
#
# NOT re-exported here: a harness's switch.  `get_backend` is
# `kathryn.sim.backend.cocotb.get_backend`, so a caller always names the harness.

from .base import SimBackend, read_version_line

__all__ = ["SimBackend", "read_version_line"]
