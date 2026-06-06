# Flow blocks as context managers. `__enter__` opens the scope (initialize) so
# components/sub-blocks created inside attach to it; `__exit__` finalizes and
# builds it. Nesting is tracked by the arena's internal stacks, not here.

from . import _session
from .signal import to_ref


class _FlowBlockCtx:
    __slots__ = ("_ident",)

    def __init__(self, ident):
        self._ident = ident

    @property
    def ident(self):
        return self._ident

    def __enter__(self):
        _session.arena().initialize_flow_block(self._ident)
        return self

    def __exit__(self, exc_type, exc, tb):
        # Leave the half-built block alone if the body raised.
        if exc_type is not None:
            return False
        _session.arena().finalize_flow_block(self._ident)
        return False


def _block(prefix, make, name, *args):
    return _FlowBlockCtx(make(name or _session.auto_name(prefix), *args))


# ---- sequential / parallel --------------------------------------------------
def seq        (name=None): return _block("seq",        _session.arena().mk_flow_block_seq,         name)
def par_auto   (name=None): return _block("par",        _session.arena().mk_flow_block_par_auto,    name)
def par_no_sync(name=None): return _block("par",        _session.arena().mk_flow_block_par_no_sync, name)

# ---- conditional (combinational / sequential if-elif-else) ------------------
def cif   (cond, name=None): return _block("cif",    _session.arena().mk_flow_block_cif,    name, to_ref(cond)._ident)
def sif   (cond, name=None): return _block("sif",    _session.arena().mk_flow_block_sif,    name, to_ref(cond)._ident)
def cselif(cond, name=None): return _block("cselif", _session.arena().mk_flow_block_cselif, name, to_ref(cond)._ident)
def cselse(      name=None): return _block("cselse", _session.arena().mk_flow_block_cselse, name)

# ---- zero-cycle conditional -------------------------------------------------
def zif   (cond, name=None): return _block("zif",    _session.arena().mk_flow_block_zif,    name, to_ref(cond)._ident)
def zelif (cond, name=None): return _block("zelif",  _session.arena().mk_flow_block_zelif,  name, to_ref(cond)._ident)
def zelse (      name=None): return _block("zelse",  _session.arena().mk_flow_block_zelse,  name)

# ---- zero-cycle switch ------------------------------------------------------
def zstate(state,     name=None): return _block("zstate", _session.arena().mk_flow_block_zstate, name, to_ref(state)._ident)
def zcase (match_val, name=None): return _block("zcase",  _session.arena().mk_flow_block_zcase,  name, int(match_val))

# ---- loops ------------------------------------------------------------------
def cwhile  (cond,          name=None): return _block("cwhile",  _session.arena().mk_flow_block_cwhile,       name, to_ref(cond)._ident)
def swhile  (cond,          name=None): return _block("swhile",  _session.arena().mk_flow_block_swhile,       name, to_ref(cond)._ident)
def cdowhile(cond,          name=None): return _block("cdowhile", _session.arena().mk_flow_block_do_while,    name, to_ref(cond)._ident)
def cloop   (last_loop_cnt, name=None): return _block("cloop", _session.arena().mk_flow_block_counter_loop, name, int(last_loop_cnt))
