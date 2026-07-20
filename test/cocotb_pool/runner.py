# The run loop: build + simulate the pooled cases and collect their results.

from __future__ import annotations

import os
import re
import sys
import shutil
import pathlib
import importlib

from . import paths
from .registry  import TestCase, pool
from .backend   import get_backend
from .discovery import (
    DiscoveredCase,
    discover_testcases,
    module_description,
    toplevel_from_verilog,
    status_from_results,
)
from .summary import CaseResult, print_summary


def run_all(simulator: str = "icarus") -> list[CaseResult]:
    return _run_cases(pool(), simulator)


def run_selected(names: list[str], simulator: str = "icarus") -> list[CaseResult]:
    cases = [tc for tc in pool() if tc.name in names]
    if not cases:
        raise RuntimeError(f"no registered cases matched: {names!r} — registered: {[tc.name for tc in pool()]}")
    return _run_cases(cases, simulator)


def _run_cases(cases: list[TestCase], simulator: str) -> list[CaseResult]:
    # Build + simulate the given cases. A tc module may hold several
    # `@cocotb.test()` coroutines; each is simulated in its own run so it dumps
    # its own VCD under <OUT_ROOT>/<name>/<testcase>.vcd. Build and simulation
    # failures are captured (not raised) so every case runs and the run ends with
    # one summary table.

    # The sim subprocess imports both kathryn and the tc module — make both findable.
    extra_path = os.pathsep.join([str(paths.PY_DIR), str(paths.MODEL), *paths.EXTRA_PYTHONPATH])
    os.environ["PYTHONPATH"] = os.pathsep.join(
        p for p in (extra_path, os.environ.get("PYTHONPATH", "")) if p)

    if not cases:
        raise RuntimeError("no test cases registered — import the tc* modules first")

    results: list[CaseResult] = []

    for tc in cases:
        out = paths.OUT_ROOT / tc.name
        out.mkdir(parents=True, exist_ok=True)
        discovered = discover_testcases(tc.test_module)

        # 1. kathryn model build + verilog emit, then cocotb compile. Any failure
        #    here (model panic or iverilog error) marks the whole case COMPILE.
        try:
            tc.build_fn(str(out))
            sources   = sorted(str(p) for p in out.glob("*.v"))
            toplevel  = toplevel_from_verilog(out / "top.v")
            backend   = get_backend(simulator)
            runner    = backend.make_runner()
            build_dir = out / "sim_build"
            runner.build(
                verilog_sources = sources,
                hdl_toplevel    = toplevel,
                build_dir       = str(build_dir),
                build_args      = backend.build_args(),
                always          = True,
                waves           = True,
                timescale       = ("1ns", "1ps"),
            )
        except Exception as e:                       # noqa: BLE001 — report any build failure
            detail = f"{type(e).__name__}: {e}".splitlines()[0]
            print(f"[{tc.name}] build/compile failed: {detail}", file=sys.stderr)
            rows = discovered or [DiscoveredCase(tc.name, module_description(tc.test_module), False)]
            for dc in rows:
                results.append(CaseResult(f"{tc.name}.{dc.name}", dc.description, "COMPILE", detail))
            continue

        # 2. simulate each testcase on its own so each dumps a separate VCD.
        for dc in discovered:
            ident = f"{tc.name}.{dc.name}"
            if dc.skip:
                results.append(CaseResult(ident, dc.description, "SKIP"))
                continue

            # absolute results file so we can classify pass/fail regardless of cwd.
            rxml = (build_dir / f"{dc.name}.results.xml").resolve()
            try:
                rxml.unlink()
            except OSError:
                pass

            try:
                runner.test(
                    hdl_toplevel = toplevel,
                    test_module  = tc.test_module,
                    testcase     = dc.name,
                    build_dir    = str(build_dir),
                    results_xml  = str(rxml),
                    waves        = True,
                    timescale    = ("1ns", "1ps"),
                )
            except SystemExit:
                pass    # cocotb exits non-zero on a failing test; status comes from the xml
            except Exception as e:                   # noqa: BLE001 — unexpected harness error
                print(f"[{ident}] test harness error: {e}", file=sys.stderr)

            results.append(CaseResult(ident, dc.description, status_from_results(rxml)))

            # The dump path is baked into the compiled sim, so each run rewrites the
            # same VCD; copy it out under the testcase name before the next run.
            # The file name is simulator-specific (icarus → <toplevel>.vcd,
            # verilator → dump.vcd), so ask the backend.
            produced = build_dir / backend.waves_file(toplevel)
            if produced.exists():
                shutil.copyfile(produced, out / f"{dc.name}.vcd")

    print_summary(results)
    return results


def discover_and_run(simulator: str = "icarus", names: list[str] | None = None) -> list[CaseResult]:
    # Import every tc*.py under test/model (registering each), then run the pool.
    # If `names` is given, only simulate those cases (all are still imported so
    # their @cocotb.test() coroutines are available to the sim subprocess).
    for p in (str(paths.MODEL), str(paths.PY_DIR), *paths.EXTRA_PYTHONPATH):
        if p not in sys.path:
            sys.path.insert(0, p)

    # Natural sort: order by the numeric index in `tcN_...` so tc2 precedes tc10
    # (plain lexicographic sort would yield tc1, tc10, tc2, ...).
    def _tc_index(f: pathlib.Path) -> tuple[int, str]:
        m = re.match(r"tc(\d+)", f.stem)
        return (int(m.group(1)) if m else 1 << 30, f.stem)

    for f in sorted(paths.MODEL.glob("tc*.py"), key=_tc_index):
        importlib.import_module(f.stem)

    if names:
        return run_selected(names, simulator)
    return run_all(simulator)
