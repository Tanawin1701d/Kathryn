# Asm-node priority controls. The priority applied to an assignment is read at
# `gen_basic_assign` time (the Rust update-event factory calls get_asm_pri_val),
# so set the priority BEFORE the assignment it should govern. State is a
# process-wide thread-local in Rust; everything routes through the one arena.

from __future__ import annotations

from types import TracebackType
from typing import Optional

from . import _session
from . import _kathryn

# Mirror every UE-priority constant from the extension into this module's globals,
# driven by the authoritative name list the Rust binding publishes. No constant
# name is hardcoded on the Python side, so adding a row to the host macro flows
# through to `from kathryn import DEFAULT_UE_...` automatically.
PRIORITY_CONST_NAMES = list(_kathryn._ASM_PRIORITY_CONST_NAMES)
globals().update({n: getattr(_kathryn, n) for n in PRIORITY_CONST_NAMES})


# ---- raw setters / getters --------------------------------------------------
def set_priority(p: int) -> None:
    # Pin every subsequent assignment to manual priority `p` until changed.
    _session.arena().set_asm_pri_to_manual(int(p))

def set_priority_auto() -> None:
    # Return to auto mode (priority resets to DEFAULT_UE_PRI_USER).
    _session.arena().set_asm_pri_to_auto()

def get_priority() -> int:
    # The priority value applied to subsequently-built update events.
    return _session.arena().get_asm_pri_val()

def get_priority_mode() -> str:
    # "Auto" or "Manual".
    return _session.arena().get_asm_pri_mode()


# ---- scoped override --------------------------------------------------------
class priority:
    """Context manager: set manual priority on enter, restore the previous
    mode/value on exit. `with priority(100): r |= a` governs only that assign."""

    __slots__ = ("_p", "_prev_mode", "_prev_val")

    def __init__(self, p: int) -> None:
        self._p = int(p)

    def __enter__(self) -> "priority":
        self._prev_mode = get_priority_mode()
        self._prev_val  = get_priority()
        set_priority(self._p)
        return self

    def __exit__(
        self,
        exc_type: Optional[type[BaseException]],
        exc     : Optional[BaseException],
        tb      : Optional[TracebackType],
    ) -> bool:
        # Restore exactly what was active before, whether auto or a manual value.
        if self._prev_mode == "Auto":
            set_priority_auto()
        else:
            set_priority(self._prev_val)
        return False
