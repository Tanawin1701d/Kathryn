# Shared cocotb test pool + Makefile-free runner.
#
# Every `tcN_*.py` file under `test/model/` registers itself here at import time
# via `register(...)`: it supplies (1) a `build_fn` that constructs the kathryn
# model and emits Verilog into a per-case output dir, and (2) its own module name
# (where the `@cocotb.test()` coroutines live). `run_all()` then loops the pool,
# builds each DUT with the cocotb Python runner API (no Makefile), and simulates
# it, dumping a VCD per case.
#
# The implementation is split across this package for readability:
#   paths.py      — path constants + configure() (mutable pool config)
#   backend.py    — SimBackend abstraction + get_backend() factory
#   icarus.py     — IcarusVCD runner subclass + IcarusBackend
#   verilator.py  — verilator tool discovery + VerilatorBackend
#   registry.py   — TestCase pool + register()/pool()
#   discovery.py  — verilog/testcase introspection + results classification
#   summary.py    — CaseResult + the summary table
#   runner.py     — the build + simulate loop (run_all / run_selected / discover_and_run)

from __future__ import annotations

from .paths      import configure
from .registry   import TestCase, register, pool
from .discovery  import DiscoveredCase, toplevel_from_verilog, discover_testcases
from .backend    import SimBackend, get_backend
from .summary    import CaseResult, print_summary
from .runner     import run_all, run_selected, discover_and_run

__all__ = [
    "configure",
    "TestCase", "register", "pool",
    "DiscoveredCase", "toplevel_from_verilog", "discover_testcases",
    "SimBackend", "get_backend",
    "CaseResult", "print_summary",
    "run_all", "run_selected", "discover_and_run",
]


def __getattr__(name):
    # Keep mutable config (MODEL / OUT_ROOT / EXTRA_PYTHONPATH / path constants)
    # live so `cocotb_pool.OUT_ROOT` reflects the current value even after a
    # configure() call rebinds it inside the paths module.
    from . import paths
    try:
        return getattr(paths, name)
    except AttributeError:
        raise AttributeError(f"module {__name__!r} has no attribute {name!r}") from None
