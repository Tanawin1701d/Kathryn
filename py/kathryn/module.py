# `Module` — a base class the user extends, marking methods with `@init`
# (hardware declaration) and `@flow` (flow-block construction). Instantiating the
# subclass opens the module scope, runs the `@init` methods then the `@flow`
# methods (both inside the scope, so `self.x = reg(...)` and the flow blocks
# attach to this module), then finalizes it. The top module is the arena itself.

from . import _session


_INIT_PHASE = "init"
_FLOW_PHASE = "flow"


# The @init / @flow decorators don't wrap the method — they just *tag* the
# function object with a `_kathryn_phase` attribute (a plain string). The tag is
# what _phase_methods() below looks for to decide which methods to call, and when.
def init(fn):
    # Mark a method as a hardware-declaration phase method (runs first, inside scope).
    fn._kathryn_phase = _INIT_PHASE
    return fn


def flow(fn):
    # Mark a method as a flow-block construction phase method (runs after @init).
    fn._kathryn_phase = _FLOW_PHASE
    return fn


def _phase_methods(cls, phase):
    # Return the *names* of methods tagged with this phase (see @init / @flow),
    # ordered so that base-class methods come before derived ones and each name
    # appears once. `Module.__init__` then calls them via getattr(self, name)().
    #
    # Two pieces of Python introspection do the work:
    #   - cls.__mro__       : the class's inheritance chain, e.g. for
    #                         `class B(A)` where `A(Module)` it is
    #                         (B, A, Module, object). reversed(...) makes us visit
    #                         the oldest ancestor first, so an inherited @init runs
    #                         before the subclass's own — matching declaration order.
    #   - klass.__dict__    : the methods/attributes defined *directly* on one
    #                         class (NOT inherited ones). Keys are names ('my_init'),
    #                         values are the function objects. It preserves the order
    #                         the methods were written in the class body. That's why
    #                         we loop the MRO and read each class's own __dict__,
    #                         rather than reading one combined dict.
    #
    # `name` is the method name string; `attr` is the function object the decorator
    # tagged. getattr(attr, "_kathryn_phase", None) reads that tag back (None for
    # untagged entries like '__module__'). `seen` dedupes so an overridden method is
    # listed once; getattr(self, name) later resolves to the most-derived override.
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
    def __init__(self, name=None):
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
    def ident(self):
        return self._ident


def set_top(module):
    # Public entry: hand a user-built Module to the session to register as the
    # design's top (delegates to the private _session plumbing). Call once, after
    # constructing the top Module, before gen_flow / build_flow.
    return _session._set_top(module)
