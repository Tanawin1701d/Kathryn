# Introspection helpers: find the @cocotb.test() coroutines in a tc module and
# describe them. (The top-module name and the junit status now come from
# kathryn.sim.runner_cocotb.)

from __future__ import annotations

import importlib
from dataclasses import dataclass

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
