# `Module` — a base class the user extends, marking methods with `@init`
# (hardware declaration) and `@flow` (flow-block construction). Instantiating the
# subclass opens the module scope, runs the `@init` methods then the `@flow`
# methods (both inside the scope, so `self.x = reg(...)` and the flow blocks
# attach to this module), then finalizes it. The top module is the arena itself.

from __future__ import annotations

from typing import Any, Callable, List, Optional, TypeVar

from . import _session
from ._kathryn import ModuleIdent


_INIT_PHASE = "init"
_FLOW_PHASE = "flow"

# Decorators are IDENTITY functions, so the decorated method keeps its own
# signature for callers and type checkers.
PhaseFn = TypeVar("PhaseFn", bound=Callable[..., Any])


# @init / @flow do NOT wrap the method — they TAG the function object with a
# `_kathryn_phase` string that _phase_methods() reads back. Wrapping would hide
# the method's signature and break `getattr(self, name)()` dispatch.
def init(fn: PhaseFn) -> PhaseFn:
    # Mark a method as a hardware-declaration phase method (runs first, inside scope).
    setattr(fn, "_kathryn_phase", _INIT_PHASE)      # setattr: a bare Callable has no such field
    return fn


def flow(fn: PhaseFn) -> PhaseFn:
    # Mark a method as a flow-block construction phase method (runs after @init).
    setattr(fn, "_kathryn_phase", _FLOW_PHASE)
    return fn


def _phase_methods(cls: type, phase: str) -> List[str]:
    # NAMES of the methods tagged with `phase`, in the order they must run.
    # `Module.__init__` calls them back via getattr(self, name)().
    #
    # - reversed(cls.__mro__): oldest ancestor FIRST, so an inherited @init runs
    #   before the subclass's own.
    # - each class's OWN __dict__ (not a merged one): it preserves class-body
    #   declaration order, which a combined dict would lose.
    # - `seen` dedupes an override to one entry; getattr later resolves it to the
    #   MOST-DERIVED binding, so a subclass override wins at its base's position.
    seen, out = set(), []
    for klass in reversed(cls.__mro__):
        for name, attr in klass.__dict__.items():
            if getattr(attr, "_kathryn_phase", None) == phase and name not in seen:
                seen.add(name)
                out.append(name)
    return out


class Module:
    # Extend this and decorate methods with @init / @flow.
    #
    # Two-phase construction:
    #   - @init runs EAGERLY in __init__, once, inside a single module scope, so
    #     hardware (`self.x = reg(...)`) is declared into the module up front.
    #   - @flow is DEFERRED. The bound flow methods are NOT called at construction;
    #     each registers into ONE process-wide pool (_session.flow_pool) keyed by
    #     this module's ident. A single top-level _session.gen_flow() then builds
    #     every module's flows from that pool — there is no per-instance build.
    def __init__(self, name: Optional[str] = None) -> None:
        name        = name or _session.auto_name(type(self).__name__)
        self._ident = _session.arena().mk_module(name)
        arena       = _session.arena()

        # ---- init phase: eager, one scope -----------------------------------
        arena.track_module_at_com_init(self._ident)
        try:
            for m in _phase_methods(type(self), _INIT_PHASE):
                getattr(self, m)()
        finally:
            arena.untrack_module_at_com_init(self._ident)

        # ---- flow phase: deferred — register into the global pool -----------
        for m in _phase_methods(type(self), _FLOW_PHASE):
            _session.register_flow(self._ident, getattr(self, m))

    @property
    def ident(self) -> ModuleIdent:
        return self._ident


def set_top(module: Module) -> Module:
    # Public entry: hand a user-built Module to the session to register as the
    # design's top (delegates to the private _session plumbing). Call once, after
    # constructing the top Module, before gen_flow / build_flow.
    return _session._set_top(module)
