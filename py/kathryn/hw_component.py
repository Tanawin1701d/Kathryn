# One lowercase class per user-constructible hardware component. Each constructor
# calls the matching arena `mk_*` and keeps only the returned ident. `name` is
# always optional and last; the |= vs *= guard is derived from the ident's own
# sensitivity class (see SignalRef._clocked), not passed in here.

from __future__ import annotations

from typing import Optional

from . import _session
from .signal import SignalRef, to_ref


class reg(SignalRef):
    __slots__ = ()
    def __init__(self, bit_width: int, name: Optional[str] = None) -> None:
        name  = name or _session.auto_name("reg")
        ident = _session.arena().mk_reg(name, int(bit_width))
        super().__init__(ident)


class wire(SignalRef):
    __slots__ = ()
    def __init__(self, bit_width: int, name: Optional[str] = None) -> None:
        name  = name or _session.auto_name("wire")
        ident = _session.arena().mk_wire(name, int(bit_width))
        super().__init__(ident)


class val(SignalRef):
    __slots__ = ()
    def __init__(self, bit_width: int, init_val: int, name: Optional[str] = None) -> None:
        name  = name or _session.auto_name("val")
        ident = _session.arena().mk_val(name, int(bit_width), int(init_val))
        super().__init__(ident)  # constant: not assignable (read-only ident)


class mem_blk(SignalRef):
    __slots__ = ()
    def __init__(self, bit_width: int, index_width: int, name: Optional[str] = None) -> None:
        name  = name or _session.auto_name("mem_blk")
        ident = _session.arena().mk_mem_blk(name, int(bit_width), int(index_width))
        super().__init__(ident)


class mem_ele(SignalRef):
    __slots__ = ()
    def __init__(
        self,
        master_mem_blk: SignalRef,
        index         : SignalRef,
        bit_width     : int,
        is_read       : bool,
        name          : Optional[str] = None,
    ) -> None:
        name  = name or _session.auto_name("mem_ele")
        ident = _session.arena().mk_mem_ele(
            name, to_ref(master_mem_blk)._ident, to_ref(index)._ident,
            int(bit_width), bool(is_read))
        super().__init__(ident)
