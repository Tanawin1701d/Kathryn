# Arbiter (Arb CCP) leaf helpers. The arb itself is created with arena().mk_arb;
# these add contention leaves to it. Each leaf owns a 1-bit req/ack wire pair,
# unless one channel is hard-tied to constant 1 (see ArbLockedChannel).

from __future__ import annotations

from . import _session
from ._kathryn import ArbLockedChannel


# ---- arb leaves -------------------------------------------------------------
def arb_add_leaf(arb, priority: int) -> int:
    # Add a leaf with its own req/ack wires; returns the leaf index.
    return _session.arena().arb_add_leaf(arb, int(priority))

def arb_add_leaf_locked(arb, priority: int, channel: ArbLockedChannel) -> int:
    # Add a leaf with one channel hard-tied to 1 (ArbLockedChannel.Req → always
    # requesting; .Ack → always granted); returns the leaf index.
    return _session.arena().arb_add_leaf_locked(arb, int(priority), int(channel))
