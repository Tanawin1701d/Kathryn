# Dynamic counter (DynCounter CCP) DSL surface. `counter` owns only the Rust
# CcpIdent; every operation routes back through the singleton arena. Each
# `add(k, en)` chains one combinational stage `en ? prev + k : prev` onto the
# counter; `update()` commits the chain head into the backing register as one
# clocked statement in the current flow scope (the chain then restarts from the
# register). The value wraps mod 2**bit_width.

from __future__ import annotations

from typing import Optional, Union

from .. import _session
from ..hw_component import val
from ..signal import SignalRef, to_ref


class counter:
    """Accumulating counter: `add(amount, enable)` stages a conditional add
    (unconditional without `enable`); `update()` commits the staged chain into
    the register. Read `.value` (committed) or `.now` (chain head, comb)."""

    __slots__ = ("_ident", "_width")

    def __init__(self, bit_width: int, name: Optional[str] = None) -> None:
        name        = name or _session.auto_name("counter")
        self._width = int(bit_width)
        self._ident = _session.arena().mk_dyn_counter(name, self._width)

    # ---- identity ----------------------------------------------------------
    @property
    def ident(self):        return self._ident       # the underlying CcpIdent
    @property
    def width(self) -> int: return self._width

    # ---- views -------------------------------------------------------------
    @property
    def value(self) -> SignalRef:
        # The committed register (last update, visible after the clock edge).
        return SignalRef(_session.arena().dyn_counter_get_reg(self._ident))

    @property
    def now(self) -> SignalRef:
        # Head of the uncommitted add chain (the register when nothing pending).
        return SignalRef(_session.arena().dyn_counter_get_now(self._ident))

    # ---- operations ---------------------------------------------------------
    def add(self, amount: Union[SignalRef, int], enable: Optional[SignalRef] = None) -> SignalRef:
        # Chain one stage: `enable ? prev + amount : prev`. An int amount is
        # wrapped into a counter-width constant.
        amt = to_ref(amount) if isinstance(amount, SignalRef) else val(self._width, int(amount))
        if enable is None:
            head = _session.arena().dyn_counter_add(self._ident, amt._ident, amt._slice)
        else:
            en   = to_ref(enable)
            head = _session.arena().dyn_counter_add(self._ident, amt._ident, amt._slice,
                                                    en._ident, en._slice)
        return SignalRef(head)

    def update(self) -> None:
        # Commit the chain head into the register (one clocked statement here).
        _session.arena().dyn_counter_update(self._ident)

    def reset(self, value: Union[SignalRef, int]) -> None:
        # Reset value of the backing register (mirrors reg.reset).
        self.value.reset(value)
