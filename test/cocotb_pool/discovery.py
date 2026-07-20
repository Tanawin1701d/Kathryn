# Introspection helpers: pull the top module name out of emitted Verilog, find
# the @cocotb.test() coroutines in a tc module, and classify a JUnit results file.

from __future__ import annotations

import re
import importlib
import pathlib
from dataclasses import dataclass
from xml.etree import ElementTree

from . import paths


def toplevel_from_verilog(top_v: pathlib.Path) -> str:
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


def module_description(test_module: str) -> str:
    # Best-effort one-line description: the first non-empty line of the module's
    # leading `#` comment block (tc files document themselves there).
    path = paths.MODEL / f"{test_module}.py"
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


def discover_testcases(test_module: str) -> list[DiscoveredCase]:
    # A `@cocotb.test()`-decorated coroutine becomes a `cocotb._decorators.Test`
    # instance carrying a `.name`. Return them in definition order so each can be
    # simulated on its own and dump a separate VCD, each tagged with a description.
    from cocotb._decorators import Test

    mod      = importlib.import_module(test_module)
    mod_desc = module_description(test_module)
    out: list[DiscoveredCase] = []
    for obj in vars(mod).values():
        if not isinstance(obj, Test):
            continue
        doc  = (getattr(obj.func, "__doc__", None) or getattr(obj, "doc", None) or "").strip()
        desc = doc.splitlines()[0].strip() if doc else mod_desc
        out.append(DiscoveredCase(obj.name, desc, bool(getattr(obj, "skip", False))))
    return out


def status_from_results(results_xml: pathlib.Path) -> str:
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
