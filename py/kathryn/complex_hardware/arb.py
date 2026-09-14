# Arbiter (Arb CCP) DSL surface. `arb` is the class object users hold; it owns
# only the Rust CcpIdent and routes every operation back through the singleton
# arena. Leaves own a 1-bit req/ack wire pair (unless one channel is hard-tied to
# constant 1 — see ArbLockedChannel); the build pass wires the combinational
# arbitration graph automatically once the enclosing module is built.

from __future__ import annotations

from typing import Optional

from .. import _session
from .._kathryn import ArbSamePriPolicy, CcpIdent, HcpIdent
from ..hw_component import val, wire
from ..signal import SignalRef, to_ref


# ---- arb leaf ---------------------------------------------------------------
class ArbLeaf:
    """One arbiter client: its request input and ack output, plus its index.

    `req` is a combinational wire the user drives to contend (`leaf.req *= cond`);
    `ack` is the grant the build pass drives (read it). A locked channel is a
    const 1 instead of a wire, so don't drive that side.
    """

    __slots__ = ("index", "req", "ack")
    index : int         # leaf position, as allocated by add_leaf
    req   : SignalRef   # 1-bit: USER drives it
    ack   : SignalRef   # 1-bit: the BUILD PASS drives it — read only

    def __init__(self, index: int, req: SignalRef, ack: SignalRef) -> None:
        self.index = index
        self.req   = req
        self.ack   = ack


# ---- helpers ------------------------------------------------------------------
def _opt_ref(hcp_i: Optional[HcpIdent]) -> Optional[SignalRef]:
    # An unbound gate comes back as None; a bound one as a whole-signal ref.
    return SignalRef(hcp_i) if hcp_i is not None else None


# ---- arb --------------------------------------------------------------------
class Arb:
    """Priority arbiter (Arb CCP). Add leaves, optionally bind a master-ack gate
    plus hold/reset signals; the host build pass wires the graph. `policy` is a
    `kathryn.ArbSamePriPolicy` member resolving same-priority ties."""

    __slots__ = ("_ident",)
    _ident : CcpIdent

    # `policy` is an ArbSamePriPolicy member (AckAll / AckOne / NotAck). Typed
    # `int` because the enum is BUILT AT RUNTIME from Rust — an IntEnum a static
    # checker cannot resolve. Same for `channel: ArbLockedChannel` below.
    def __init__(self, policy: int = ArbSamePriPolicy.AckOne, name: Optional[str] = None) -> None:
        name        = name or _session.auto_name("arb")
        self._ident = _session.arena().mk_arb(name, int(policy))

    # ---- identity ----------------------------------------------------------
    @property
    def ident(self) -> CcpIdent: return self._ident   # pip / zync take this

    @property
    def leaf_count(self) -> int: return _session.arena().arb_leaf_count(self._ident)

    @property
    def master_req(self) -> SignalRef:
        # OR of every leaf request (1-bit wire), readable once leaves are added.
        return SignalRef(_session.arena().arb_get_master_req_wire(self._ident))

    # ---- bound gate sources (None until bound) ------------------------------
    # - `master_ack` is bound by a pip DURING build_flow (None before), or is the
    #   const 1 of no_pip_master();
    # - hold / reset are bound at set_hold / set_reset time. An unsliced signal
    #   resolves to the SAME ident: `con.hold.global_id == hold_wire.global_id`.
    @property
    def master_ack(self) -> Optional[SignalRef]: return _opt_ref(_session.arena().arb_get_master_ack(self._ident))
    @property
    def hold      (self) -> Optional[SignalRef]: return _opt_ref(_session.arena().arb_get_hold      (self._ident))
    @property
    def reset     (self) -> Optional[SignalRef]: return _opt_ref(_session.arena().arb_get_reset     (self._ident))

    # ---- leaves ------------------------------------------------------------
    def add_leaf(self, priority: int) -> ArbLeaf:
        # Add a leaf with its own req/ack wires; returns its handle.
        idx = _session.arena().arb_add_leaf(self._ident, int(priority))
        return self.leaf(idx)

    def add_leaf_locked(self, priority: int, channel: int) -> ArbLeaf:
        # Add a leaf with one channel hard-tied to 1 (ArbLockedChannel.Req → always
        # requesting; .Ack → always granted); returns its handle.
        idx = _session.arena().arb_add_leaf_locked(self._ident, int(priority), int(channel))
        return self.leaf(idx)

    def leaf(self, idx: int) -> ArbLeaf:
        # The leaf at `idx`, as add_leaf returned it: read its req/ack after the fact.
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


    # ---- convenience constant drivers --------------------------------------
    def stall(self) -> SignalRef:
        # Drive a fresh 1-bit wire to constant 1 and bind it as the hold gate.
        hold_wire = wire(1, "arb_stall")
        hold_wire *= val(1, 1, "arb_hold_val")
        self.set_hold(hold_wire)
        return hold_wire

    def flush(self) -> SignalRef:
        # Drive a fresh 1-bit wire to constant 1 and bind it as the reset gate.
        flush_wire = wire(1, "arb_flush")
        flush_wire *= val(1, 1, "arb_flush_val")
        self.set_reset(flush_wire)
        return flush_wire
