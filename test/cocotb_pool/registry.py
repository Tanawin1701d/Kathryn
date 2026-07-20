# The shared test-case pool. Each `tcN_*.py` calls `register(...)` at import
# time to add itself; `pool()` hands the collected cases to the runner.

from __future__ import annotations

from dataclasses import dataclass
from typing import Callable


@dataclass
class TestCase:
    name        : str                        # unique case id, also its output sub-dir
    build_fn    : Callable[[str], None]      # build kathryn model + emit verilog into dir
    test_module : str                        # python module holding @cocotb.test() coroutines


_POOL: list[TestCase] = []


def register(name: str, build_fn: Callable[[str], None], test_module: str) -> None:
    # Idempotent on name — cocotb re-imports the tc module inside the sim process,
    # which would otherwise double-register.
    if any(tc.name == name for tc in _POOL):
        return
    _POOL.append(TestCase(name, build_fn, test_module))


def pool() -> list[TestCase]:
    return list(_POOL)
