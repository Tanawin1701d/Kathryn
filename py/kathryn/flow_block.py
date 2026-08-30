# Flow blocks as context managers. `__enter__` opens the scope (initialize) so
# components/sub-blocks created inside attach to it; `__exit__` finalizes and
# builds it. Nesting is tracked by the arena's internal stacks, not here.
# NOT here: the arbiter-gated blocks (`pip` / `zync`) live in pip_zync.py — they
# reuse _FlowBlockCtx / _block / _complex_block from this file.

from __future__ import annotations

from types import TracebackType
from typing import Any, Callable, Optional, Tuple

from . import _session
from ._kathryn import FlowBlockType, FlowBlockIdent, HcpIdent, Slice
from .signal import Source, to_ref


class _FlowBlockCtx:
    # Flow block as a context manager. `__enter__` opens the scope (initialize);
    # `__exit__` finalizes it. Complex blocks (conditional / loop) can't own nodes
    # directly, so when `is_req_auto_sub_blk` we auto-open an inner skeleton (par_auto / seq)
    # matching the enclosing skeleton — the body's nodes attach to it — and finalize
    # that inner skeleton first on exit.
    __slots__ = ("_ident", "_is_req_auto_sub_blk", "_inner_i")
    _ident               : FlowBlockIdent
    _is_req_auto_sub_blk : bool
    _inner_i             : Optional[FlowBlockIdent]   # the auto-opened skeleton, if any

    def __init__(self, ident: FlowBlockIdent, is_req_auto_sub_blk: bool = False) -> None:
        self._ident              = ident
        self._is_req_auto_sub_blk = is_req_auto_sub_blk
        self._inner_i            = None

    @property
    def ident(self) -> FlowBlockIdent:
        return self._ident

    def __enter__(self) -> _FlowBlockCtx:
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

    def __exit__(
        self,
        exc_type: Optional[type[BaseException]],
        exc     : Optional[BaseException],
        tb      : Optional[TracebackType],
    ) -> bool:
        # Leave the half-built block(s) alone if the body raised.
        if exc_type is not None:
            return False
        arena = _session.arena()

        # finalize first, then check: finalize_flow_block folds in any lingering
        # lazy-closed child (a trailing if/elif chain) via its recursion, so the
        # block is complete when validated instead of appearing empty. Finalize only
        # attaches (no build), so it never panics on the constraint we are checking.

        # automatic subblock finalize and check
        if self._inner_i is not None:
            arena.finalize_flow_block(self._inner_i)
            arena.check_flow_block_prefinalize(self._inner_i)

        # main block finalize and check
        arena.finalize_flow_block(self._ident)
        arena.check_flow_block_prefinalize(self._ident)
        return False


# `*args` is the chosen factory's OWN tail, so it varies per block type:
#   (HcpIdent, Slice)               conditions / loops / waits — see _cond_args
#   int                             zcase match value, cloop count, sywait cycles
#   HcpIdent                        zstate switch variable
#   (CcpIdent, priority, bool, bool)   pip
#   (binds, match_all, priority)       zync
# Typed `Any` because no single signature covers all of them.
def _block(
    prefix: str,
    make  : Callable[..., FlowBlockIdent],
    name  : Optional[str],
    *args : Any,
) -> _FlowBlockCtx:
    return _FlowBlockCtx(make(name or _session.auto_name(prefix), *args))

def _complex_block(
    prefix: str,
    make  : Callable[..., FlowBlockIdent],
    name  : Optional[str],
    *args : Any,
) -> _FlowBlockCtx:
    return _FlowBlockCtx(make(name or _session.auto_name(prefix), *args), is_req_auto_sub_blk=True)

def _leaf_block(
    prefix: str,
    make  : Callable[..., FlowBlockIdent],
    name  : Optional[str],
    *args : Any,
) -> FlowBlockIdent:
    # Leaf blocks (wait) own no body, so they are not context managers: create,
    # open, validate, and immediately close so the block attaches to the
    # enclosing scope as a single statement.
    arena   = _session.arena()
    block_i = make(name or _session.auto_name(prefix), *args)
    arena.initialize_flow_block(block_i)
    arena.check_flow_block_prefinalize(block_i)
    arena.finalize_flow_block(block_i)
    return block_i


# ---- sequential / parallel --------------------------------------------------
def seq        (name: Optional[str] = None) -> _FlowBlockCtx: return _block("seq", _session.arena().mk_flow_block_seq,         name)
def par        (name: Optional[str] = None) -> _FlowBlockCtx: return _block("par", _session.arena().mk_flow_block_par_auto,    name)
def par_auto   (name: Optional[str] = None) -> _FlowBlockCtx: return _block("par", _session.arena().mk_flow_block_par_auto,    name)
def par_no_sync(name: Optional[str] = None) -> _FlowBlockCtx: return _block("par", _session.arena().mk_flow_block_par_no_sync, name)

# Thread a condition signal through as (ident, slice): a sliced SignalRef (e.g.
# cond[3, 0]) carries the partial range, which the host wraps in a SliceBit
# expression when it doesn't cover the whole variable.
def _cond_args(cond: Source) -> Tuple[HcpIdent, Slice]:
    r = to_ref(cond)
    return (r._ident, r._slice)

# ---- conditional (combinational / sequential if-elif-else) ------------------
# Complex blocks — an inner skeleton (seq/par) is auto-opened (is_req_auto_sub_blk).
def cif   (cond: Source, name: Optional[str] = None) -> _FlowBlockCtx: return _complex_block("cif",    _session.arena().mk_flow_block_cif,    name, *_cond_args(cond))
def sif   (cond: Source, name: Optional[str] = None) -> _FlowBlockCtx: return _complex_block("sif",    _session.arena().mk_flow_block_sif,    name, *_cond_args(cond))
def cselif(cond: Source, name: Optional[str] = None) -> _FlowBlockCtx: return _complex_block("cselif", _session.arena().mk_flow_block_cselif, name, *_cond_args(cond))
def cselse(              name: Optional[str] = None) -> _FlowBlockCtx: return _complex_block("cselse", _session.arena().mk_flow_block_cselse, name)

# ---- zero-cycle conditional -------------------------------------------------
def zif   (cond: Source, name: Optional[str] = None) -> _FlowBlockCtx: return _block("zif",   _session.arena().mk_flow_block_zif,   name, *_cond_args(cond))
def zelif (cond: Source, name: Optional[str] = None) -> _FlowBlockCtx: return _block("zelif", _session.arena().mk_flow_block_zelif, name, *_cond_args(cond))
def zelse (              name: Optional[str] = None) -> _FlowBlockCtx: return _block("zelse", _session.arena().mk_flow_block_zelse, name)

# ---- zero-cycle switch ------------------------------------------------------
def zstate(state: Source,  name: Optional[str] = None) -> _FlowBlockCtx: return _block("zstate", _session.arena().mk_flow_block_zstate, name, to_ref(state)._ident)
def zcase (match_val: int, name: Optional[str] = None) -> _FlowBlockCtx: return _block("zcase",  _session.arena().mk_flow_block_zcase,  name, int(match_val))

# ---- pick (container + pif / pidef branches) --------------------------------
# `pick` runs whichever `pif` branch matches its raw condition (no chaining — keep
# the conditions mutually exclusive yourself). The optional `pidef` runs only when
# no pif matched. WARNING: the pick exit is NOT auto-synchronized — the branch that
# fires drives the exit signal. `pif`/`pidef` are complex blocks (auto inner skeleton).
def pick (              name: Optional[str] = None) -> _FlowBlockCtx: return _block("pick", _session.arena().mk_flow_block_pick, name)
def pif  (cond: Source, name: Optional[str] = None) -> _FlowBlockCtx: return _complex_block("pif",   _session.arena().mk_flow_block_pif,   name, *_cond_args(cond))
def pidef(              name: Optional[str] = None) -> _FlowBlockCtx: return _complex_block("pidef", _session.arena().mk_flow_block_pidef, name)

# ---- loops (complex blocks — inner skeleton auto-opened) --------------------
def cwhile  (cond: Source,       name: Optional[str] = None) -> _FlowBlockCtx: return _complex_block("cwhile",   _session.arena().mk_flow_block_cwhile,       name, *_cond_args(cond))
def swhile  (cond: Source,       name: Optional[str] = None) -> _FlowBlockCtx: return _complex_block("swhile",   _session.arena().mk_flow_block_swhile,       name, *_cond_args(cond))
def cdowhile(cond: Source,       name: Optional[str] = None) -> _FlowBlockCtx: return _complex_block("cdowhile", _session.arena().mk_flow_block_do_while,     name, *_cond_args(cond))
def cloop   (last_loop_cnt: int, name: Optional[str] = None) -> _FlowBlockCtx: return _complex_block("cloop",    _session.arena().mk_flow_block_counter_loop, name, int(last_loop_cnt))

# ---- waits (leaf blocks — no body, run as a statement) ----------------------
def scwait(cond: Source, name: Optional[str] = None) -> FlowBlockIdent: return _leaf_block("scwait", _session.arena().mk_flow_block_scwait, name, *_cond_args(cond))
def sywait(cycle: int,   name: Optional[str] = None) -> FlowBlockIdent: return _leaf_block("sywait", _session.arena().mk_flow_block_sywait, name, int(cycle))