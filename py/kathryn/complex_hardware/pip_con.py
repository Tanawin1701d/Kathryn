# PipCon — the arbiter handle a `pip` / `zync` flow block contends on. It is an
# `arb` (inherits the whole arbiter surface: leaves, master-ack/hold/reset), so a
# PipCon can be configured exactly like a plain arb. The block-side auto leaf
# (pip → Req-locked, zync → Ack-locked) is added on the host side when the block
# is created — not here — so PipCon itself adds nothing beyond the arb identity.

from __future__ import annotations

from .arb import Arb


class PipCon(Arb):
    """Arbiter metadata for pip/zync blocks. A thin `arb` subclass: hand one to
    `pip(...)` or `zync(...)` and the host auto-adds the block's locked leaf."""

    __slots__ = ()
