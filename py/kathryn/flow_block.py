# Flow blocks as context managers. `__enter__` opens the scope (initialize) so
# components/sub-blocks created inside attach to it; `__exit__` finalizes and
# builds it. Nesting is tracked by the arena's internal stacks, not here.

from __future__ import annotations

from types import TracebackType
from typing import Callable, Optional

from . import _session
from ._kathryn import FlowBlockType, FlowBlockIdent
from .complex_hardware import PipCon
from .signal import SignalRef, to_ref


class _FlowBlockCtx:
    # Flow block as a context manager. `__enter__` opens the scope (initialize);
    # `__exit__` finalizes it. Complex blocks (conditional / loop) can't own nodes
    # directly, so when `is_req_auto_sub_blk` we auto-open an inner skeleton (par_auto / seq)
    # matching the enclosing skeleton — the body's nodes attach to it — and finalize
    # that inner skeleton first on exit.
    __slots__ = ("_ident", "_is_req_auto_sub_blk", "_inner_i")

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


def _block(
    prefix: str,
    make  : Callable[..., FlowBlockIdent],
    name  : Optional[str],
    *args : object,
) -> _FlowBlockCtx:
    return _FlowBlockCtx(make(name or _session.auto_name(prefix), *args))

def _complex_block(
    prefix: str,
    make  : Callable[..., FlowBlockIdent],
    name  : Optional[str],
    *args : object,
) -> _FlowBlockCtx:
    return _FlowBlockCtx(make(name or _session.auto_name(prefix), *args), is_req_auto_sub_blk=True)

def _leaf_block(
    prefix: str,
    make  : Callable[..., FlowBlockIdent],
    name  : Optional[str],
    *args : object,
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
def _cond_args(cond: SignalRef) -> tuple:
    r = to_ref(cond)
    return (r._ident, r._slice)

# ---- conditional (combinational / sequential if-elif-else) ------------------
# Complex blocks — an inner skeleton (seq/par) is auto-opened (is_req_auto_sub_blk).
def cif   (cond: SignalRef, name: Optional[str] = None) -> _FlowBlockCtx: return _complex_block("cif",    _session.arena().mk_flow_block_cif,    name, *_cond_args(cond))
def sif   (cond: SignalRef, name: Optional[str] = None) -> _FlowBlockCtx: return _complex_block("sif",    _session.arena().mk_flow_block_sif,    name, *_cond_args(cond))
def cselif(cond: SignalRef, name: Optional[str] = None) -> _FlowBlockCtx: return _complex_block("cselif", _session.arena().mk_flow_block_cselif, name, *_cond_args(cond))
def cselse(                 name: Optional[str] = None) -> _FlowBlockCtx: return _complex_block("cselse", _session.arena().mk_flow_block_cselse, name)

# ---- zero-cycle conditional -------------------------------------------------
def zif   (cond: SignalRef, name: Optional[str] = None) -> _FlowBlockCtx: return _block("zif",   _session.arena().mk_flow_block_zif,   name, *_cond_args(cond))
def zelif (cond: SignalRef, name: Optional[str] = None) -> _FlowBlockCtx: return _block("zelif", _session.arena().mk_flow_block_zelif, name, *_cond_args(cond))
def zelse (                 name: Optional[str] = None) -> _FlowBlockCtx: return _block("zelse", _session.arena().mk_flow_block_zelse, name)

# ---- zero-cycle switch ------------------------------------------------------
def zstate(state: SignalRef,   name: Optional[str] = None) -> _FlowBlockCtx: return _block("zstate", _session.arena().mk_flow_block_zstate, name, to_ref(state)._ident)
def zcase (match_val: int,     name: Optional[str] = None) -> _FlowBlockCtx: return _block("zcase",  _session.arena().mk_flow_block_zcase,  name, int(match_val))

# ---- pick (container + pif / pidef branches) --------------------------------
# `pick` runs whichever `pif` branch matches its raw condition (no chaining — keep
# the conditions mutually exclusive yourself). The optional `pidef` runs only when
# no pif matched. WARNING: the pick exit is NOT auto-synchronized — the branch that
# fires drives the exit signal. `pif`/`pidef` are complex blocks (auto inner skeleton).
def pick (                 name: Optional[str] = None) -> _FlowBlockCtx: return _block("pick", _session.arena().mk_flow_block_pick, name)
def pif  (cond: SignalRef, name: Optional[str] = None) -> _FlowBlockCtx: return _complex_block("pif",   _session.arena().mk_flow_block_pif,   name, *_cond_args(cond))
def pidef(                 name: Optional[str] = None) -> _FlowBlockCtx: return _complex_block("pidef", _session.arena().mk_flow_block_pidef, name)

# ---- loops (complex blocks — inner skeleton auto-opened) --------------------
def cwhile  (cond: SignalRef,      name: Optional[str] = None) -> _FlowBlockCtx: return _complex_block("cwhile",   _session.arena().mk_flow_block_cwhile,       name, *_cond_args(cond))
def swhile  (cond: SignalRef,      name: Optional[str] = None) -> _FlowBlockCtx: return _complex_block("swhile",   _session.arena().mk_flow_block_swhile,       name, *_cond_args(cond))
def cdowhile(cond: SignalRef,      name: Optional[str] = None) -> _FlowBlockCtx: return _complex_block("cdowhile", _session.arena().mk_flow_block_do_while,     name, *_cond_args(cond))
def cloop   (last_loop_cnt: int,   name: Optional[str] = None) -> _FlowBlockCtx: return _complex_block("cloop",    _session.arena().mk_flow_block_counter_loop, name, int(last_loop_cnt))

# ---- waits (leaf blocks — no body, run as a statement) ----------------------
def scwait(cond: SignalRef,    name: Optional[str] = None) -> FlowBlockIdent: return _leaf_block("scwait", _session.arena().mk_flow_block_scwait, name, *_cond_args(cond))
def sywait(cycle: int,         name: Optional[str] = None) -> FlowBlockIdent: return _leaf_block("sywait", _session.arena().mk_flow_block_sywait, name, int(cycle))

# pip/zync contend on a shared arbiter — it must be a `PipCon`, not a plain arb or
# raw ident, so the locked-leaf contract the host relies on is guaranteed.
def _pip_con_ident(meta) -> "object":
    if not isinstance(meta, PipCon):
        raise TypeError(f"pip/zync `meta` must be a PipCon, got {type(meta).__name__}")
    return meta.ident

# ---- pipeline (complex block — inner skeleton auto-opened) -------------------
# Gated by `meta` (a PipCon). The host adds the pip's leaf at `priority`:
# `auto_req=False` (default) is a normal leaf; `auto_req=True` Req-locks it (always
# requesting). `auto_restart` routes the arb user-reset into the block's start
# signal so a reset re-launches the pipeline instead of clearing it.
def pip(meta, name: Optional[str] = None, *, auto_restart: bool = False, priority: Optional[int] = None, auto_req: bool = False) -> _FlowBlockCtx:
    return _complex_block("pip", _session.arena().mk_flow_block_pip, name,
                          _pip_con_ident(meta), priority, auto_req, auto_restart)

# ---- zync (plain block — owns its work asm nodes directly) -------------------
# Contend on one OR several arbiters. `meta` is a single bind or a list of binds;
# each bind is one of:
#     PipCon                       -> contend on it, no condition, default priority
#     (PipCon, cond)               -> gate this arb on the 1-bit `cond` SignalRef
#     (PipCon, cond, priority)     -> also pin this arb's leaf arbitration priority
# A bind's `cond` gates both its REQ (`state_exit & cond`) and its grant term
# (`ack & cond`); pass `None` for an always-on bind. `mode` selects how the binds'
# grants combine: "any" (default) fires when ANY bind's `ack & cond` is high (OR);
# "all" fires only when EVERY bind's is high (AND). For a single bind the two coincide.
# `priority` is the default leaf priority for binds that don't pin their own;
# `auto_ack` Ack-locks every bind's leaf (always granted) — default is a normal leaf.
# WARNING (mode="all"): the grant is AND over `ack & cond`, so a bind whose condition
# is false contributes 0 — the target may fire on a partial set of the arbs.
def _check_opt_priority(p, where: str):
    # Priorities are ints (or None to inherit the default); reject bool/float/etc.
    if p is not None and (not isinstance(p, int) or isinstance(p, bool)):
        raise TypeError(f"{where} priority must be an int or None, got {type(p).__name__}")
    return p

def _zync_one_bind(item, auto_ack: bool) -> tuple:
    # Normalize one bind into (pip_ident, priority|None, auto_ack, cond_ident|None).
    if isinstance(item, tuple):
        if not 1 <= len(item) <= 3:
            raise TypeError("zync bind tuple must be (meta), (meta, cond) or "
                            f"(meta, cond, priority); got a {len(item)}-tuple")
        meta     = item[0]
        cond     = item[1] if len(item) >= 2 else None
        priority = item[2] if len(item) >= 3 else None
    else:
        meta, cond, priority = item, None, None

    pip_i  = _pip_con_ident(meta)                                # raises unless a PipCon
    cond_i = to_ref(cond)._ident if cond is not None else None   # raises unless a signal
    _check_opt_priority(priority, "zync bind")
    return (pip_i, priority, bool(auto_ack), cond_i)

def _zync_binds(meta, auto_ack: bool) -> list:
    items = meta if isinstance(meta, list) else [meta]
    if not items:
        raise ValueError("zync requires at least one arb bind")
    return [_zync_one_bind(it, auto_ack) for it in items]

def _zync_match_all(mode) -> bool:
    norm = mode.lower() if isinstance(mode, str) else mode
    if norm == "all":           return True
    if norm in ("some", "any"): return False
    raise ValueError(f"zync mode must be 'all' or 'some', got {mode!r}")

def zync(
    meta,
    name    : Optional[str] = None,
    *,
    mode    : str           = "any",
    priority: Optional[int] = None,
    auto_ack: bool          = False,
) -> _FlowBlockCtx:
    _check_opt_priority(priority, "zync")
    return _block("zync", _session.arena().mk_flow_block_zync_multi, name,
                  _zync_binds(meta, auto_ack), _zync_match_all(mode), priority)