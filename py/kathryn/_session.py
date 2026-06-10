# The one ModelArena the whole DSL builds into. Created once at import; Python's
# module cache makes this a process-wide singleton, so even repeated
# `import kathryn` reuse the same arena (ownership stays entirely in Rust).

from __future__ import annotations

from typing import TYPE_CHECKING, Callable

from ._kathryn import ModelArena, ModuleIdent, BackendVerilog

if TYPE_CHECKING:
    from .module import Module


def _make_arena() -> ModelArena:
    # Build an empty arena with NO module open. The top module is explicit: the
    # user must call `set_top(...)` before declaring any hardware so component
    # factories have a module on the trace stack to attach to.
    return ModelArena()

def auto_name(prefix: str) -> str:
    # Per-prefix monotonic name (reg0, reg1, ...) used when the user omits a name.
    # Rust still appends the global_id, so collisions are impossible regardless.
    n = _counters.get(prefix, 0)
    _counters[prefix] = n + 1
    return f"{prefix}{n}"


def _set_top(module: Module) -> Module:
    # Register a user-built `Module` (see module.py) as the design's top, then
    # re-open its scope so subsequent top-level components / flow blocks attach to
    # it. The module's `@init` hardware is declared by its own constructor; this
    # only records its ident as top (the build DFS starts there) and keeps the
    # top scope active. Call once, after constructing the top Module.
    top_i = module.ident
    a     = arena()
    a.set_top_module(top_i)
    a.track_module_at_com_init(top_i)
    return module


_arena    : ModelArena                                = _make_arena()
_counters : dict[str, int]                            = {}
# Process-wide deferred-flow pool: every Module's @flow methods register here as
# (module_ident, bound_method) so a single gen_flow() can build them all. (See
# module.py — flow construction is deferred, not run at instantiation.)
_flow_pool: list[tuple[ModuleIdent, Callable[[], None]]] = []


def arena() -> ModelArena:
    # Always call this per-operation (never cache the object) so `reset` is visible.
    return _arena


def reset() -> ModelArena:
    # Rebuild the arena from scratch with NO top module (call `set_top` after);
    # mainly for tests. Clears auto-name counters and the deferred-flow pool.
    global _arena, _counters, _flow_pool
    _arena     = _make_arena()
    _counters  = {}
    _flow_pool = []
    return _arena


def register_flow(module_i: ModuleIdent, fn: Callable[[], None]) -> None:
    # Append one module's deferred @flow method to the global pool.
    _flow_pool.append((module_i, fn))


def flow_pool() -> list[tuple[ModuleIdent, Callable[[], None]]]:
    # The global (module_ident, bound_method) pool, in registration order.
    return _flow_pool


def gen_flow() -> None:
    # Build EVERY module's deferred @flow methods from the one global pool, in
    # registration order. Each call re-opens its own module scope (so
    # track_module_at_flow_init / untrack_module_at_flow_init are paired per flow
    # call and run many times across modules). Non-consuming: safe to invoke more
    # than once.
    for module_i, flow_fun in _flow_pool:
        _arena.track_module_at_flow_init(module_i)
        try:
            flow_fun()
        finally:
            _arena.untrack_module_at_flow_init(module_i)


def build_flow() -> None:
    # Run the host build pass: starting from the top module, build the hardware
    # for every flow block across the module tree (schematics, update events,
    # clk / master-reset wiring). Call once, AFTER `gen_flow` has constructed all
    # flow blocks. Not re-runnable — the top build asserts a fresh start node.
    arena().build_flow()


def build_model(module: Module) -> Module:
    # One-shot convenience: register `module` as the top, construct every module's
    # deferred @flow blocks, then run the host build pass. Equivalent to
    # `set_top(module); gen_flow(); build_flow()`. Not re-runnable (see build_flow).
    _set_top(module)
    gen_flow()
    build_flow()
    return module


def emit_verilog(output_dir: str) -> None:
    # Final consumer: run the Verilog backend over the built model. Constructing
    # the backend MOVES the singleton arena into it, so the session arena is left
    # EMPTY afterwards (call `reset` + rebuild to run again). `output_dir` must
    # already exist; one `<output_dir>/<module>.v` is written per module. Call
    # after `build_flow` / `build_model`.
    BackendVerilog(arena()).emit(output_dir)
