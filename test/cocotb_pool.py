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
import sys
import shutil
import pathlib
import importlib
from dataclasses import dataclass
from typing import Callable
from xml.etree import ElementTree

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


@dataclass
class DiscoveredCase:
    name        : str    # the @cocotb.test() coroutine name
    description : str     # per-test docstring, else the module's leading comment
    skip        : bool    # cocotb @test(skip=True)


def _module_description(test_module: str) -> str:
    # Best-effort one-line description: the first non-empty line of the module's
    # leading `#` comment block (tc files document themselves there).
    path = _MODEL / f"{test_module}.py"
    try:
        for line in path.read_text().splitlines():
            s = line.strip()
            if s.startswith("#!"):              # shebang
                continue
            if s.startswith("#"):
                text = s.lstrip("#").strip()
                if text:
                    return text
            elif s == "":
                continue
            else:                               # first real code line — stop
                break
    except OSError:
        pass
    return ""


def _discover_testcases(test_module: str) -> list[DiscoveredCase]:
    # A `@cocotb.test()`-decorated coroutine becomes a `cocotb._decorators.Test`
    # instance carrying a `.name`. Return them in definition order so each can be
    # simulated on its own and dump a separate VCD, each tagged with a description.
    from cocotb._decorators import Test

    mod      = importlib.import_module(test_module)
    mod_desc = _module_description(test_module)
    out: list[DiscoveredCase] = []
    for obj in vars(mod).values():
        if not isinstance(obj, Test):
            continue
        doc  = (getattr(obj.func, "__doc__", None) or getattr(obj, "doc", None) or "").strip()
        desc = doc.splitlines()[0].strip() if doc else mod_desc
        out.append(DiscoveredCase(obj.name, desc, bool(getattr(obj, "skip", False))))
    return out


def _status_from_results(results_xml: pathlib.Path) -> str:
    # Classify a single-testcase JUnit results file: PASS, FAIL, or NO_RESULT
    # (file missing / unparsable → the sim crashed before writing results).
    if not results_xml.is_file():
        return "NO_RESULT"
    try:
        tree = ElementTree.parse(results_xml)
    except ElementTree.ParseError:
        return "NO_RESULT"
    n_tc = n_bad = 0
    for tc in tree.iter("testcase"):
        n_tc += 1
        if any(True for _ in tc.iter("failure")) or any(True for _ in tc.iter("error")):
            n_bad += 1
    if n_tc == 0:
        return "NO_RESULT"
    return "FAIL" if n_bad else "PASS"


def _make_runner(simulator: str):
    # icarus is the default; we use a VCD-emitting subclass (cocotb 2.x defaults
    # icarus waves to FST). Any other simulator falls back to cocotb's stock runner.
    if simulator == "icarus":
        return _IcarusVCD()
    from cocotb_tools.runner import get_runner
    return get_runner(simulator)


@dataclass
class CaseResult:
    case        : str    # "<tc>.<testcase>" identifier
    description : str
    status      : str     # PASS / FAIL / COMPILE / NO_RESULT / SKIP
    detail      : str = ""  # error message for COMPILE / NO_RESULT rows


# Status → (symbol, ANSI colour) for the summary table.
_STATUS_STYLE = {
    "PASS"     : ("PASS",       "\033[32m"),  # green
    "FAIL"     : ("FAIL",       "\033[31m"),  # red
    "COMPILE"  : ("COMPILE-ERR", "\033[35m"),  # magenta
    "NO_RESULT": ("NO-RESULT",  "\033[33m"),  # yellow
    "SKIP"     : ("SKIP",       "\033[90m"),  # grey
}
_RESET = "\033[0m"


def run_all(simulator: str = "icarus") -> list["CaseResult"]:
    return _run_cases(pool(), simulator)


def run_selected(names: list[str], simulator: str = "icarus") -> list["CaseResult"]:
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
    extra_path = os.pathsep.join([str(_PY_DIR), str(_MODEL)])
    os.environ["PYTHONPATH"] = os.pathsep.join(
        p for p in (extra_path, os.environ.get("PYTHONPATH", "")) if p)

    if not cases:
        raise RuntimeError("no test cases registered — import the tc* modules first")

    results: list[CaseResult] = []

    for tc in cases:
        out = OUT_ROOT / tc.name
        out.mkdir(parents=True, exist_ok=True)
        discovered = _discover_testcases(tc.test_module)

        # 1. kathryn model build + verilog emit, then cocotb compile. Any failure
        #    here (model panic or iverilog error) marks the whole case COMPILE.
        try:
            tc.build_fn(str(out))
            sources   = sorted(str(p) for p in out.glob("*.v"))
            toplevel  = _toplevel_from_verilog(out / "top.v")
            runner    = _make_runner(simulator)
            build_dir = out / "sim_build"
            runner.build(
                verilog_sources = sources,
                hdl_toplevel    = toplevel,
                build_dir       = str(build_dir),
                always          = True,
                waves           = True,
                timescale       = ("1ns", "1ps"),
            )
        except Exception as e:                       # noqa: BLE001 — report any build failure
            detail = f"{type(e).__name__}: {e}".splitlines()[0]
            print(f"[{tc.name}] build/compile failed: {detail}", file=sys.stderr)
            rows = discovered or [DiscoveredCase(tc.name, _module_description(tc.test_module), False)]
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

            results.append(CaseResult(ident, dc.description, _status_from_results(rxml)))

            # The dump path is baked into the compiled sim, so each run rewrites the
            # same VCD; copy it out under the testcase name before the next run.
            produced = build_dir / f"{toplevel}.vcd"
            if produced.exists():
                shutil.copyfile(produced, out / f"{dc.name}.vcd")

    _print_summary(results)
    return results


def _print_summary(results: list[CaseResult]) -> None:
    use_color = sys.stdout.isatty()

    def styled(status: str) -> tuple[str, str]:
        label, color = _STATUS_STYLE.get(status, (status, ""))
        return label, (color if use_color else "")

    # column widths (status uses the plain label width, colour codes excluded)
    status_w = max([len("STATUS")] + [len(styled(r.status)[0]) for r in results] or [len("STATUS")])
    case_w   = max([len("TEST CASE")] + [len(r.case) for r in results])
    desc_w   = min(60, max([len("DESCRIPTION")] + [len(r.description) for r in results]))

    def trunc(s: str, w: int) -> str:
        return s if len(s) <= w else s[: w - 1] + "…"

    sep  = f"+-{'-'*status_w}-+-{'-'*case_w}-+-{'-'*desc_w}-+"
    head = f"| {'STATUS':<{status_w}} | {'TEST CASE':<{case_w}} | {'DESCRIPTION':<{desc_w}} |"

    print()
    print("=" * len(sep))
    print(" TEST SUMMARY")
    print(sep)
    print(head)
    print(sep)
    for r in results:
        label, color = styled(r.status)
        status_cell = f"{color}{label:<{status_w}}{_RESET if color else ''}"
        desc = trunc(r.description, desc_w)
        print(f"| {status_cell} | {r.case:<{case_w}} | {desc:<{desc_w}} |")
        if r.detail:
            print(f"| {'':<{status_w}} | {'└─ ' + trunc(r.detail, case_w + desc_w):<{case_w + desc_w + 3}} |")
    print(sep)

    counts = {k: sum(1 for r in results if r.status == k) for k in _STATUS_STYLE}
    total  = len(results)
    summary = (
        f" {counts['PASS']} passed, {counts['FAIL']} failed, "
        f"{counts['COMPILE']} compile-err, {counts['NO_RESULT']} no-result, "
        f"{counts['SKIP']} skipped   ({total} total)"
    )
    print(summary)
    print("=" * len(sep))


def discover_and_run(simulator: str = "icarus", names: list[str] | None = None) -> list[CaseResult]:
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
        return run_selected(names, simulator)
    return run_all(simulator)
