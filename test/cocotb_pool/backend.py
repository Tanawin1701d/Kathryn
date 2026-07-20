# Simulator backend abstraction.
#
# Everything that differs between simulators lives on a `SimBackend` subclass —
# how to construct the cocotb runner, what extra compile flags it needs, and the
# name of the VCD the compiled sim actually writes. `runner.py` talks only to this
# interface, so adding a simulator is a new file + one line in the registry below.

from __future__ import annotations


class SimBackend:
    # Generic fallback backend: stock cocotb runner, no extra flags, <toplevel>.vcd.
    def __init__(self, name: str):
        self.name = name

    def build_args(self) -> list[str]:
        # Extra flags passed to runner.build(build_args=...).
        return []

    def waves_file(self, toplevel: str) -> str:
        # Name of the dump the compiled sim writes into build_dir (each backend
        # owns this, since the filename is simulator-specific).
        return f"{toplevel}.vcd"

    def make_runner(self):
        # A ready-to-use cocotb runner instance (may resolve/validate the tool first).
        from cocotb_tools.runner import get_runner
        return get_runner(self.name)


_REGISTRY: dict[str, SimBackend] | None = None


def get_backend(name: str) -> SimBackend:
    # Resolve a simulator name to its backend; unknown names fall back to the
    # generic stock-runner backend. Backends are stateless, so instances are cached.
    global _REGISTRY
    if _REGISTRY is None:
        from .icarus    import IcarusBackend
        from .verilator import VerilatorBackend
        _REGISTRY = {b.name: b for b in (IcarusBackend(), VerilatorBackend())}
    return _REGISTRY.get(name, SimBackend(name))
