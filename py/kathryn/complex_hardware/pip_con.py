# PipCon — the arbiter handle a `pip` / `zync` flow block contends on. It is an
# `arb` (inherits the whole arbiter surface: leaves, master-ack/hold/reset), so a
# PipCon can be configured exactly like a plain arb. The block-side auto leaf
# (pip → Req-locked, zync → Ack-locked) is added on the host side when the block
# is created — not here — so PipCon itself adds nothing beyond the arb identity.

from __future__ import annotations

from .. import _session
from .arb import Arb


class PipCon(Arb):
    """Arbiter metadata for pip/zync blocks. A thin `arb` subclass: hand one to
    `pip(...)` or `zync(...)` and the host auto-adds the block's locked leaf."""

    __slots__ = ()

    def no_pip_master(self) -> None:
        # Declare that NO pip block masters this PipCon: the master-ack gate is
        # hard-tied to constant 1, so a zync leaf is granted the moment it wins
        # arbitration (a lone zync's ack simply mirrors its req).
        # - call inside a module scope (@init), like the PipCon constructor;
        # - a pip built on this PipCon afterwards fails the set-once assert.
        _session.arena().arb_lock_master_ack(self._ident)
