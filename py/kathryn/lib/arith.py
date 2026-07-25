# Widening arithmetic helpers built on the signed/unsigned extend primitives.
# The DSL's `*` is width-preserving (result = left operand width), so a full
# product needs both operands pre-extended to the result width — mod-2^N
# arithmetic then makes the extended unsigned multiply exact for every
# signedness combination. Helpers declare wires, so call them from @flow.

from __future__ import annotations

from typing import Union

from .. import _session
from ..hw_component import wire
from ..signal import SignalRef, to_ref
from .bits import width_of, zext, sext


def mul_wide(
    a       : SignalRef,
    b       : SignalRef,
    a_signed: bool = True,
    b_signed: bool = True,
    name    : Union[str, None] = None,
) -> SignalRef:
    # Full (wa+wb)-bit product. Each operand is sign- or zero-extended to the
    # product width first; the plain unsigned multiply of the extended values is
    # then exact mod 2^(wa+wb), which covers MUL/MULH/MULHU/MULHSU semantics.
    a  = to_ref(a)
    b  = to_ref(b)
    w2 = width_of(a) + width_of(b)
    ax = sext(a, w2, name=name and f"{name}_a") if a_signed else zext(a, w2)
    bx = sext(b, w2, name=name and f"{name}_b") if b_signed else zext(b, w2)
    return ax * bx


def mulh(
    a       : SignalRef,
    b       : SignalRef,
    a_signed: bool = True,
    b_signed: bool = True,
    name    : Union[str, None] = None,
) -> SignalRef:
    # High half of the full product (RISC-V MULH / MULHU / MULHSU), as a wire of
    # the common operand width. Requires equal operand widths.
    a = to_ref(a)
    b = to_ref(b)
    w = width_of(a)
    if width_of(b) != w:
        raise ValueError(f"mulh: operand widths differ ({w} vs {width_of(b)})")
    prod = mul_wide(a, b, a_signed, b_signed, name=name)
    out  = wire(w, name or _session.auto_name("mulh"))
    out *= prod[2 * w - 1, w]
    return out
