# Introspection helpers: find the @cocotb.test() coroutines in a tc module and
# describe them. (The top-module name and the junit status now come from
# kathryn.sim.runner_cocotb.)

from __future__ import annotations

import importlib
from dataclasses import dataclass
from typing import Any

from . import paths


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
    # The module's cocotb tests (each a `cocotb._decorators.Test` carrying a
    # `.name`) in definition order, so each can be simulated on its own and
    # dump a separate VCD, each tagged with a description.
    mod      = importlib.import_module(test_module)
    mod_desc = module_description(test_module)
    out: list[DiscoveredCase] = []
    for obj in vars(mod).values():
        for test in list_cocotb_tests(obj):
            doc  = (getattr(test.func, "__doc__", None) or getattr(test, "doc", None) or "").strip()
            desc = doc.splitlines()[0].strip() if doc else mod_desc
            out.append(DiscoveredCase(test.name, desc, bool(getattr(test, "skip", False))))
    return out


def list_cocotb_tests(obj: Any) -> list[Any]:
    """The `Test`s one module attribute stands for, else [].

    - cocotb 2.0's @cocotb.test() IS a Test; 2.1's is a TestGenerator (and
      2.0's @parametrize a Parameterized), which generate their Tests
    """
    from cocotb import _decorators
    test_types = tuple(t for t in (getattr(_decorators, name, None)
                                   for name in ("Test", "Parameterized", "TestGenerator"))
                       if t is not None)
    if not isinstance(obj, test_types):
        return []
    return list(obj.generate_tests()) if hasattr(obj, "generate_tests") else [obj]
