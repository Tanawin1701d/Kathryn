# The one ModelArena the whole DSL builds into. Created once at import; Python's
# module cache makes this a process-wide singleton, so even repeated
# `import kathryn` reuse the same arena (ownership stays entirely in Rust).

from ._kathryn import ModelArena

_DEFAULT_TOP = "top"


def _make_arena(top_name):
    # Build an empty arena, then create + register the top module and open it so
    # component factories always have a module on the trace stack to attach to.
    # `mk_module` reads its parent off the trace-stack top — empty here, so the
    # top module gets no parent. `initialize_module` pushes it at FlowBlockInit.
    a = ModelArena()
    top_i = a.mk_module(top_name)
    a.set_top_module(top_i)
    a.initialize_module(top_i)
    return a


_arena     = _make_arena(_DEFAULT_TOP)
_counters  = {}
# Process-wide deferred-flow pool: every Module's @flow methods register here as
# (module_ident, bound_method) so a single gen_flow() can build them all. (See
# module.py — flow construction is deferred, not run at instantiation.)
_flow_pool = []


def arena():
    # Always call this per-operation (never cache the object) so `reset` is visible.
    return _arena


def reset(top_name=_DEFAULT_TOP):
    # Rebuild the arena from scratch (mainly for tests); clears auto-name counters
    # and the deferred-flow pool.
    global _arena, _counters, _flow_pool
    _arena     = _make_arena(top_name)
    _counters  = {}
    _flow_pool = []
    return _arena


def register_flow(module_i, fn):
    # Append one module's deferred @flow method to the global pool.
    _flow_pool.append((module_i, fn))


def flow_pool():
    # The global (module_ident, bound_method) pool, in registration order.
    return _flow_pool


def gen_flow():
    # Build EVERY module's deferred @flow methods from the one global pool, in
    # registration order. Each call re-opens its own module scope (so
    # initialize_module / finalize_module are paired per flow call and run many
    # times across modules). Non-consuming: safe to invoke more than once.
    for module_i, fn in _flow_pool:
        _arena.initialize_module(module_i)
        try:
            fn()
        finally:
            _arena.finalize_module(module_i)


def auto_name(prefix):
    # Per-prefix monotonic name (reg0, reg1, ...) used when the user omits a name.
    # Rust still appends the global_id, so collisions are impossible regardless.
    n = _counters.get(prefix, 0)
    _counters[prefix] = n + 1
    return f"{prefix}{n}"
