# The one ModelArena the whole DSL builds into. Created once at import; Python's
# module cache makes this a process-wide singleton, so even repeated
# `import kathryn` reuse the same arena (ownership stays entirely in Rust).

from ._kathryn import ModelArena

_DEFAULT_TOP = "top"

_arena    = ModelArena(_DEFAULT_TOP)
_counters = {}


def arena():
    # Always call this per-operation (never cache the object) so `reset` is visible.
    return _arena


def reset(top_name=_DEFAULT_TOP):
    # Rebuild the arena from scratch (mainly for tests); clears auto-name counters.
    global _arena, _counters
    _arena    = ModelArena(top_name)
    _counters = {}
    return _arena


def auto_name(prefix):
    # Per-prefix monotonic name (reg0, reg1, ...) used when the user omits a name.
    # Rust still appends the global_id, so collisions are impossible regardless.
    n = _counters.get(prefix, 0)
    _counters[prefix] = n + 1
    return f"{prefix}{n}"
