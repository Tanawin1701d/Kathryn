# One lowercase class per user-constructible hardware component. Each constructor
# calls the matching arena `mk_*` and keeps only the returned ident. `name` is
# always optional and last; the |= vs *= guard is derived from the ident's own
# sensitivity class (see SignalRef._clocked), not passed in here.

from __future__ import annotations

from typing import List, Optional

from . import _session
from ._kathryn import Slice
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


_U64_MASK = (1 << 64) - 1


def _int_to_limbs(n: int, bit_width: int) -> List[int]:
    # Split an arbitrary-precision int into little-endian u64 limbs, two's-complement
    # wrapped into `bit_width` bits. limbs[0] = bits 0..63, limbs[1] = 64..127, etc.
    n    &= (1 << bit_width) - 1
    words = (bit_width + 63) // 64
    return [(n >> (64 * i)) & _U64_MASK for i in range(words)]


class val(SignalRef):
    __slots__ = ()
    def __init__(self, bit_width: int, init_val: int, name: Optional[str] = None) -> None:
        name      = name or _session.auto_name("val")
        bit_width = int(bit_width)
        init_val  = int(init_val) & ((1 << bit_width) - 1)   # two's-complement wrap to width
        # ≤64-bit literals take the fast u64 path; wider ones go through limbs.
        if init_val <= _U64_MASK:
            ident = _session.arena().mk_val(name, bit_width, init_val)
        else:
            ident = _session.arena().mk_val_vv(name, bit_width, _int_to_limbs(init_val, bit_width))
        super().__init__(ident)  # constant: not assignable (read-only ident)


class mem_blk(SignalRef):
    __slots__ = ()
    def __init__(self, bit_width: int, index_width: int, name: Optional[str] = None) -> None:
        name  = name or _session.auto_name("mem_blk")
        ident = _session.arena().mk_mem_blk(name, int(bit_width), int(index_width))
        # A MemBlk is not itself an assignment destination (you read/write it via a
        # mem_ele), so it has no `get_des_slice`. Pass an explicit data-width slice so
        # SignalRef.__init__ doesn't query the arena (which would panic on the block).
        super().__init__(ident, Slice(0, int(bit_width)))


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
