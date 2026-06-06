# Sub-modules as context managers. `__enter__` opens the module scope so
# components/flow-blocks created inside attach to it; `__exit__` finalizes it
# (registering it with the enclosing module). The top module is the arena itself.

from . import _session


class _ModuleCtx:
    __slots__ = ("_ident",)

    def __init__(self, ident):
        self._ident = ident

    @property
    def ident(self):
        return self._ident

    def __enter__(self):
        _session.arena().initialize_module(self._ident)
        return self

    def __exit__(self, exc_type, exc, tb):
        if exc_type is not None:
            return False
        _session.arena().finalize_module(self._ident)
        return False


def module(name=None):
    name = name or _session.auto_name("module")
    return _ModuleCtx(_session.arena().mk_module(name))
