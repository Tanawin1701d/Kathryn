# Flow blocks as context managers. `__enter__` opens the scope (initialize) so
# components/sub-blocks created inside attach to it; `__exit__` finalizes and
# builds it. Nesting is tracked by the arena's internal stacks, not here.

from . import _session
from ._kathryn import FlowBlockType
from .signal import to_ref


class _FlowBlockCtx:
    # Flow block as a context manager. `__enter__` opens the scope (initialize);
    # `__exit__` finalizes it. Complex blocks (conditional / loop) can't own nodes
    # directly, so when `is_req_auto_sub_blk` we auto-open an inner skeleton (par_auto / seq)
    # matching the enclosing skeleton — the body's nodes attach to it — and finalize
    # that inner skeleton first on exit.
    __slots__ = ("_ident", "_is_req_auto_sub_blk", "_inner_i")

    def __init__(self, ident, is_req_auto_sub_blk=False):
        self._ident              = ident
        self._is_req_auto_sub_blk = is_req_auto_sub_blk
        self._inner_i            = None

    @property
    def ident(self):
        return self._ident

    def __enter__(self):
        arena = _session.arena()
        arena.initialize_flow_block(self._ident)

        # automatic subblock creation
        if self._is_req_auto_sub_blk:
            # Mirror the enclosing skeleton: par → par_auto, seq (or none) → seq.
            if arena.get_last_skeleton_flow_block_type() == FlowBlockType.Parallel:
                self._inner_i = arena.mk_flow_block_par_auto(_session.auto_name("par"))
            else:
                self._inner_i = arena.mk_flow_block_seq(_session.auto_name("seq"))
            arena.initialize_flow_block(self._inner_i)
        return self

    def __exit__(self, exc_type, exc, tb):
        # Leave the half-built block(s) alone if the body raised.
        if exc_type is not None:
            return False
        arena = _session.arena()

        # automatic subblock check and finalize
        if self._inner_i is not None:
            arena.check_flow_block_prefinalize(self._inner_i)
            arena.finalize_flow_block(self._inner_i)

        # main block check and finalize
        arena.check_flow_block_prefinalize(self._ident)
        arena.finalize_flow_block(self._ident)
        return False


def _block(prefix, make, name, *args):
    return _FlowBlockCtx(make(name or _session.auto_name(prefix), *args))

def _complex_block(prefix, make, name, *args):
    return _FlowBlockCtx(make(name or _session.auto_name(prefix), *args), is_req_auto_sub_blk=True)


# ---- sequential / parallel --------------------------------------------------
def seq        (name=None): return _block("seq",        _session.arena().mk_flow_block_seq,         name)
def par_auto   (name=None): return _block("par",        _session.arena().mk_flow_block_par_auto,    name)
def par_no_sync(name=None): return _block("par",        _session.arena().mk_flow_block_par_no_sync, name)

# ---- conditional (combinational / sequential if-elif-else) ------------------
# Complex blocks — an inner skeleton (seq/par) is auto-opened (is_req_auto_sub_blk).
def cif   (cond, name=None): return _complex_block("cif",    _session.arena().mk_flow_block_cif,    name, to_ref(cond)._ident)
def sif   (cond, name=None): return _complex_block("sif",    _session.arena().mk_flow_block_sif,    name, to_ref(cond)._ident)
def cselif(cond, name=None): return _complex_block("cselif", _session.arena().mk_flow_block_cselif, name, to_ref(cond)._ident)
def cselse(      name=None): return _complex_block("cselse", _session.arena().mk_flow_block_cselse, name)

# ---- zero-cycle conditional -------------------------------------------------
def zif   (cond, name=None): return _block("zif",    _session.arena().mk_flow_block_zif,    name, to_ref(cond)._ident)
def zelif (cond, name=None): return _block("zelif",  _session.arena().mk_flow_block_zelif,  name, to_ref(cond)._ident)
def zelse (      name=None): return _block("zelse",  _session.arena().mk_flow_block_zelse,  name)

# ---- zero-cycle switch ------------------------------------------------------
def zstate(state,     name=None): return _block("zstate", _session.arena().mk_flow_block_zstate, name, to_ref(state)._ident)
def zcase (match_val, name=None): return _block("zcase",  _session.arena().mk_flow_block_zcase,  name, int(match_val))

# ---- loops (complex blocks — inner skeleton auto-opened) --------------------
def cwhile  (cond,          name=None): return _complex_block("cwhile",   _session.arena().mk_flow_block_cwhile,        name, to_ref(cond)._ident)
def swhile  (cond,          name=None): return _complex_block("swhile",   _session.arena().mk_flow_block_swhile,        name, to_ref(cond)._ident)
def cdowhile(cond,          name=None): return _complex_block("cdowhile", _session.arena().mk_flow_block_do_while,      name, to_ref(cond)._ident)
def cloop   (last_loop_cnt, name=None): return _complex_block("cloop",    _session.arena().mk_flow_block_counter_loop,  name, int(last_loop_cnt))
