# Arbiter (Arb CCP) DSL surface. `arb` is the class object users hold; it owns
# only the Rust CcpIdent and routes every operation back through the singleton
# arena. Leaves own a 1-bit req/ack wire pair (unless one channel is hard-tied to
# constant 1 — see ArbLockedChannel); the build pass wires the combinational
# arbitration graph automatically once the enclosing module is built.

from __future__ import annotations

from typing import Optional

from .. import _session
from .._kathryn import ArbLockedChannel, ArbSamePriPolicy
from ..signal import SignalRef, to_ref


# ---- arb leaf ---------------------------------------------------------------
class ArbLeaf:
    """One arbiter client: its request input and ack output, plus its index.

    `req` is a combinational wire the user drives to contend (`leaf.req *= cond`);
    `ack` is the grant the build pass drives (read it). A locked channel is a
    const 1 instead of a wire, so don't drive that side.
    """

    __slots__ = ("index", "req", "ack")

    def __init__(self, index: int, req: SignalRef, ack: SignalRef) -> None:
        self.index = index
        self.req   = req
        self.ack   = ack


# ---- arb --------------------------------------------------------------------
class Arb:
    """Priority arbiter (Arb CCP). Add leaves, optionally bind a master-ack gate
    plus hold/reset signals; the host build pass wires the graph. `policy` is a
    `kathryn.ArbSamePriPolicy` member resolving same-priority ties."""

    __slots__ = ("_ident",)

    def __init__(self, policy: int = ArbSamePriPolicy.AckOne, name: Optional[str] = None) -> None:
        name        = name or _session.auto_name("arb")
        self._ident = _session.arena().mk_arb(name, int(policy))

    # ---- identity ----------------------------------------------------------
    @property
    def ident(self):     return self._ident          # the underlying CcpIdent (pip/zync take this)

    @property
    def leaf_count(self) -> int: return _session.arena().arb_leaf_count(self._ident)

    @property
    def master_req(self) -> SignalRef:
        # OR of every leaf request (1-bit wire), readable once leaves are added.
        return SignalRef(_session.arena().arb_get_master_req_wire(self._ident))

    # ---- leaves ------------------------------------------------------------
    def add_leaf(self, priority: int) -> ArbLeaf:
        # Add a leaf with its own req/ack wires; returns its handle.
        idx = _session.arena().arb_add_leaf(self._ident, int(priority))
        return self._leaf(idx)

    def add_leaf_locked(self, priority: int, channel: ArbLockedChannel) -> ArbLeaf:
        # Add a leaf with one channel hard-tied to 1 (ArbLockedChannel.Req → always
        # requesting; .Ack → always granted); returns its handle.
        idx = _session.arena().arb_add_leaf_locked(self._ident, int(priority), int(channel))
        return self._leaf(idx)

    def _leaf(self, idx: int) -> ArbLeaf:
        req_i = _session.arena().arb_get_leaf_req_wire(self._ident, idx)
        ack_i = _session.arena().arb_get_leaf_ack_wire(self._ident, idx)
        return ArbLeaf(idx, SignalRef(req_i), SignalRef(ack_i))

    # ---- optional control signals ------------------------------------------
    def set_master_ack(self, src: SignalRef) -> None:
        # Bind the single 1-bit source gating every grant (set once).
        src = to_ref(src)
        _session.arena().arb_set_master_ack(self._ident, src._ident, src._slice)

    def set_hold(self, sig: SignalRef) -> None:
        # Freeze every grant while `sig` is asserted (set once).
        sig = to_ref(sig)
        _session.arena().arb_set_hold(self._ident, sig._ident, sig._slice)

    def set_reset(self, sig: SignalRef) -> None:
        # Clear every grant while `sig` is asserted (set once).
        sig = to_ref(sig)
        _session.arena().arb_set_reset(self._ident, sig._ident, sig._slice)
