# PipCon — the arbiter handle a `pip` / `zync` flow block contends on. It is an
# `arb` (inherits the whole arbiter surface: leaves, master-ack/hold/reset), so a
# PipCon can be configured exactly like a plain arb. The block-side auto leaf
# (pip → Req-locked, zync → Ack-locked) is added on the host side when the block
# is created — not here. What PipCon adds is the RECORD of which pip masters it:
# `pip()` binds the block ident and its leaf index here (pure Python, no
# hardware), so a `@dbg` body can read the pip's status signals after build_flow.

from __future__ import annotations

from typing import Optional

from .. import _session
from .._kathryn import ArbSamePriPolicy, FlowBlockIdent
from ..signal import SignalRef
from .arb import Arb, ArbLeaf


class PipCon(Arb):
    """Arbiter metadata for pip/zync blocks. A thin `arb` subclass: hand one to
    `pip(...)` or `zync(...)` and the host auto-adds the block's locked leaf."""

    __slots__ = ("_pip_block_i", "_pip_leaf_idx")
    _pip_block_i  : Optional[FlowBlockIdent]   # the ONE pip block mastering this PipCon
    _pip_leaf_idx : Optional[int]              # that pip's leaf on this arb

    # `policy` is an ArbSamePriPolicy member; typed `int` for the reason Arb gives.
    def __init__(self, policy: int = ArbSamePriPolicy.AckOne, name: Optional[str] = None) -> None:
        super().__init__(policy, name)
        self._pip_block_i  = None
        self._pip_leaf_idx = None

    def no_pip_master(self) -> None:
        # Declare that NO pip block masters this PipCon: the master-ack gate is
        # hard-tied to constant 1, so a zync leaf is granted the moment it wins
        # arbitration (a lone zync's ack simply mirrors its req).
        # - call inside a module scope (@init), like the PipCon constructor;
        # - a pip built on this PipCon afterwards fails the set-once assert.
        _session.arena().arb_lock_master_ack(self._ident)

    # ---- the mastering pip (bound by pip(), read after build_flow) -----------
    def _bind_pip(self, block_i: FlowBlockIdent, leaf_idx: int) -> None:
        # One pip per PipCon: the host's master-ack is set-once anyway.
        if self._pip_block_i is not None:
            raise ValueError(f"PipCon {self._ident!r} already masters pip block "
                             f"{self._pip_block_i!r}; one pip per PipCon")
        self._pip_block_i  = block_i
        self._pip_leaf_idx = leaf_idx

    @property
    def pip_block_i (self) -> Optional[FlowBlockIdent]: return self._pip_block_i
    @property
    def pip_leaf_idx(self) -> Optional[int]:            return self._pip_leaf_idx

    @property
    def pip_leaf(self) -> Optional[ArbLeaf]:
        # The pip's own leaf (its REQ / grant); None when no pip masters this PipCon.
        return self.leaf(self._pip_leaf_idx) if self._pip_leaf_idx is not None else None

    @property
    def pip_wait_reg(self) -> Optional[SignalRef]:
        # The pip's wait4syn StateReg: high while the pipeline is parked on an
        # idle arb. None when no pip masters this PipCon; ValueError before build_flow.
        if self._pip_block_i is None:
            return None
        return SignalRef(_session.arena().get_pip_wait_reg(self._pip_block_i))
