# Shared cocotb test pool + Makefile-free runner.
#
# Every `tcN_*.py` file under `test/model/` registers itself here at import time
# via `register(...)`: it supplies (1) a `build_fn` that constructs the kathryn
# model and emits Verilog into a per-case output dir, and (2) its own module name
# (where the `@cocotb.test()` coroutines live). `run_all()` then loops the pool,
# builds each DUT with the cocotb Python runner API (no Makefile), and simulates
# it, dumping a VCD per case.

from __future__ import annotations

import os
import re
import shutil
import pathlib
import importlib
from dataclasses import dataclass
from typing import Callable

# ---- paths -------------------------------------------------------------------
_HERE     = pathlib.Path(__file__).resolve().parent          # .../Kathryn2/test
_REPO     = _HERE.parent                                     # .../Kathryn2
_PY_DIR   = _REPO / "py"                                     # kathryn package
_MODEL    = _HERE / "model"                                  # tc*.py live here
OUT_ROOT  = _HERE / ".model_output"                          # per-case build dirs


# cocotb 2.x's stock Icarus runner dumps FST when waves=True. We want a plain
# VCD, so override the three FST-specific spots: the dump-file path, the generated
# $dumpfile module, and the `-fst` vvp plusarg.
try:
    from cocotb_tools.runner import Icarus as _Icarus, _as_sv_literal

    class _IcarusVCD(_Icarus):
        def _waves_file(self):
            return f"{self.hdl_toplevel}.vcd"

        def _create_iverilog_dump_file(self):
            dumpfile = _as_sv_literal(str(self.build_dir / f"{self.hdl_toplevel}.vcd"))
            with open(self.iverilog_dump_file, "w") as f:
                f.write("module cocotb_iverilog_dump();\n")
                f.write("initial begin\n")
                f.write(f"    $dumpfile({dumpfile});\n")
                f.write(f"    $dumpvars(0, {self.hdl_toplevel});\n")
                f.write("end\n")
                f.write("endmodule\n")

        def _test_command(self):
            # vvp emits VCD by default (per $dumpfile extension); drop the FST flag.
            cmds = super()._test_command()
            for cmd in cmds:
                while "-fst" in cmd:
                    cmd.remove("-fst")
            return cmds
except ImportError:   # cocotb not installed / older API — only matters at run time
    _IcarusVCD = None


@dataclass
class TestCase:
    name        : str                       # unique case id, also its output sub-dir
    build_fn    : Callable[[str], None]      # build kathryn model + emit verilog into dir
    test_module : str                       # python module holding @cocotb.test() coroutines


_POOL: list[TestCase] = []


def register(name: str, build_fn: Callable[[str], None], test_module: str) -> None:
    # Idempotent on name — cocotb re-imports the tc module inside the sim process,
    # which would otherwise double-register.
    if any(tc.name == name for tc in _POOL):
        return
    _POOL.append(TestCase(name, build_fn, test_module))


def pool() -> list[TestCase]:
    return list(_POOL)


def _toplevel_from_verilog(top_v: pathlib.Path) -> str:
    # The emitted top file is `top.v`, but the module name inside is auto-generated
    # (e.g. MODULE_tc1_seq_simple0_0). cocotb needs that real module name.
    m = re.search(r"^\s*module\s+(\w+)", top_v.read_text(), re.MULTILINE)
    if not m:
        raise RuntimeError(f"no `module` declaration found in {top_v}")
    return m.group(1)


def _discover_testcases(test_module: str) -> list[str]:
    # A `@cocotb.test()`-decorated coroutine becomes a `cocotb._decorators.Test`
    # instance carrying a `.name`. Return those names in definition order so each
    # can be simulated on its own and dump a separate VCD.
    from cocotb._decorators import Test

    mod   = importlib.import_module(test_module)
    tests = [obj for obj in vars(mod).values() if isinstance(obj, Test)]
    return [t.name for t in tests]


def _make_runner(simulator: str):
    # icarus is the default; we use a VCD-emitting subclass (cocotb 2.x defaults
    # icarus waves to FST). Any other simulator falls back to cocotb's stock runner.
    if simulator == "icarus":
        return _IcarusVCD()
    from cocotb_tools.runner import get_runner
    return get_runner(simulator)


def run_all(simulator: str = "icarus") -> None:
    _run_cases(pool(), simulator)


def run_selected(names: list[str], simulator: str = "icarus") -> None:
    cases = [tc for tc in pool() if tc.name in names]
    if not cases:
        raise RuntimeError(f"no registered cases matched: {names!r} — registered: {[tc.name for tc in pool()]}")
    _run_cases(cases, simulator)


def _run_cases(cases: list[TestCase], simulator: str) -> None:
    # Build + simulate the given cases. A tc module may hold several
    # `@cocotb.test()` coroutines; each is simulated in its own run so it dumps
    # its own VCD under <OUT_ROOT>/<name>/<testcase>.vcd.

    # The sim subprocess imports both kathryn and the tc module — make both findable.
    extra_path = os.pathsep.join([str(_PY_DIR), str(_MODEL)])
    os.environ["PYTHONPATH"] = os.pathsep.join(
        p for p in (extra_path, os.environ.get("PYTHONPATH", "")) if p)

    if not cases:
        raise RuntimeError("no test cases registered — import the tc* modules first")

    for tc in cases:
        out = OUT_ROOT / tc.name
        out.mkdir(parents=True, exist_ok=True)

        # 1. kathryn: build model + emit verilog (top.v + any sub-module .v files).
        tc.build_fn(str(out))
        sources  = sorted(str(p) for p in out.glob("*.v"))
        toplevel = _toplevel_from_verilog(out / "top.v")

        # 2. cocotb runner: compile once, then simulate each testcase separately so
        #    every coroutine produces its own VCD (one shared sim would overwrite it).
        runner = _make_runner(simulator)
        build_dir = out / "sim_build"
        runner.build(
            verilog_sources = sources,
            hdl_toplevel    = toplevel,
            build_dir       = str(build_dir),
            always          = True,
            waves           = True,
            timescale       = ("1ns", "1ps"),
        )

        testcases = _discover_testcases(tc.test_module)
        for testcase in testcases:
            runner.test(
                hdl_toplevel = toplevel,
                test_module  = tc.test_module,
                testcase     = testcase,
                build_dir    = str(build_dir),
                waves        = True,
                timescale    = ("1ns", "1ps"),
            )
            # The dump path is baked into the compiled sim, so each run rewrites the
            # same VCD; copy it out under the testcase name before the next run.
            produced = build_dir / f"{toplevel}.vcd"
            if produced.exists():
                shutil.copyfile(produced, out / f"{testcase}.vcd")


def discover_and_run(simulator: str = "icarus", names: list[str] | None = None) -> None:
    # Import every tc*.py under test/model (registering each), then run the pool.
    # If `names` is given, only simulate those cases (all are still imported so
    # their @cocotb.test() coroutines are available to the sim subprocess).
    import importlib
    import sys

    if str(_MODEL) not in sys.path:
        sys.path.insert(0, str(_MODEL))
    if str(_PY_DIR) not in sys.path:
        sys.path.insert(0, str(_PY_DIR))

    # Natural sort: order by the numeric index in `tcN_...` so tc2 precedes tc10
    # (plain lexicographic sort would yield tc1, tc10, tc2, ...).
    def _tc_index(f: pathlib.Path) -> tuple[int, str]:
        m = re.match(r"tc(\d+)", f.stem)
        return (int(m.group(1)) if m else 1 << 30, f.stem)

    for f in sorted(_MODEL.glob("tc*.py"), key=_tc_index):
        importlib.import_module(f.stem)

    if names:
        run_selected(names, simulator)
    else:
        run_all(simulator)
