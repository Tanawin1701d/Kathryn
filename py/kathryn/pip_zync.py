# Arbiter-gated flow blocks: `pip` (pipeline granter half) and `zync` (requester
# half). Both contend on a shared PipCon; the generic context-manager machinery
# (_FlowBlockCtx / _block / _complex_block) stays in flow_block.py — this file
# only owns the PipCon-facing surface.

from __future__ import annotations

from typing import List, Optional, Tuple, Union

from . import _session
from ._kathryn import CcpIdent, HcpIdent
from .complex_hardware import PipCon
from .flow_block import _FlowBlockCtx, _block, _complex_block
from .signal import Source, to_ref

# One zync arb binding, in any of the accepted spellings:
#   PipCon                  contend always, default priority
#   (PipCon,)               same
#   (PipCon, cond)             gate REQ and grant on the 1-bit `cond`
#   (PipCon, cond, priority)   also pin this arb's leaf priority
ZyncBind = Union[
    PipCon,
    Tuple[PipCon],
    Tuple[PipCon, Optional[Source]],
    Tuple[PipCon, Optional[Source], Optional[int]],
]
# Normalized form handed to the connector: (arb, priority, auto_ack, cond).
ZyncBindArgs = Tuple[CcpIdent, Optional[int], bool, Optional[HcpIdent]]


# pip/zync contend on a shared arbiter — it must be a `PipCon`, not a plain arb or
# raw ident, so the locked-leaf contract the host relies on is guaranteed.
def _pip_con_ident(meta: PipCon) -> CcpIdent:
    if not isinstance(meta, PipCon):
        raise TypeError(f"pip/zync `meta` must be a PipCon, got {type(meta).__name__}")
    return meta.ident

# ---- pipeline (complex block — inner skeleton auto-opened) -------------------
# Gated by `meta` (a PipCon); the host adds the pip's leaf at `priority`.
# - `auto_req=True` Req-locks the leaf (ALWAYS requesting); default is a normal leaf.
# - `auto_restart` routes the arb user-reset into the block's start signal, so a
#   reset RE-LAUNCHES the pipeline instead of clearing it.
def pip(
    meta        : PipCon,
    name        : Optional[str] = None,
    *,
    auto_restart: bool          = False,
    priority    : Optional[int] = None,
    auto_req    : bool          = False,
) -> _FlowBlockCtx:
    return _complex_block("pip", _session.arena().mk_flow_block_pip, name,
                       _pip_con_ident(meta), priority, auto_req, auto_restart)

# ---- zync (plain block — owns its work asm nodes directly) -------------------
# Contend on one OR several arbiters. `meta` is one ZyncBind or a list of them
# (see the ZyncBind alias at the top of this file for the accepted spellings).
# - A bind's `cond` gates BOTH its REQ (`state_exit & cond`) and its grant term
#   (`ack & cond`); pass None for an always-on bind.
# - `mode` combines the binds' grants: "any" (default) = OR, "all" = AND. They
#   coincide for a single bind.
# - `priority` is the default leaf priority for binds that don't pin their own;
#   `auto_ack` Ack-locks every bind's leaf (always granted).
# WARNING (mode="all"): the grant ANDs `ack & cond`, so a bind whose condition is
# false contributes 0 — the target may fire on a PARTIAL set of the arbs.
def _check_opt_priority(p: Optional[int], where: str) -> Optional[int]:
    # Priorities are ints (or None to inherit the default); reject bool/float/etc.
    if p is not None and (not isinstance(p, int) or isinstance(p, bool)):
        raise TypeError(f"{where} priority must be an int or None, got {type(p).__name__}")
    return p

def _zync_one_bind(item: ZyncBind, auto_ack: bool) -> ZyncBindArgs:
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

    pip_i  = _pip_con_ident(meta)                             # raises unless a PipCon
    cond_i = to_ref(cond)._ident if cond is not None else None   # raises unless a signal
    _check_opt_priority(priority, "zync bind")
    return (pip_i, priority, bool(auto_ack), cond_i)

def _zync_binds(meta: Union[ZyncBind, List[ZyncBind]], auto_ack: bool) -> List[ZyncBindArgs]:
    items = meta if isinstance(meta, list) else [meta]
    if not items:
        raise ValueError("zync requires at least one arb bind")
    return [_zync_one_bind(it, auto_ack) for it in items]

def _zync_match_all(mode: object) -> bool:      # accepts non-str and raises
    norm = mode.lower() if isinstance(mode, str) else mode
    if norm == "all":           return True
    if norm in ("some", "any"): return False
    raise ValueError(f"zync mode must be 'all' or 'some', got {mode!r}")

def zync(
    meta    : Union[ZyncBind, List[ZyncBind]],
    name    : Optional[str] = None,
    *,
    mode    : str           = "any",    # "all" | "any" | "some"
    priority: Optional[int] = None,
    auto_ack: bool          = False,
) -> _FlowBlockCtx:
    _check_opt_priority(priority, "zync")
    return _block("zync", _session.arena().mk_flow_block_zync_multi, name,
               _zync_binds(meta, auto_ack), _zync_match_all(mode), priority)
