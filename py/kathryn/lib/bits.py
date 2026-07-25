# Bit-manipulation helpers the raw DSL lacks: sign-extend, concatenation,
# replication, reductions, and a 2:1 mux. Helpers that only build expressions
# (zext, or_reduce, and_reduce) are usable anywhere a signal is; helpers that
# declare a wire and/or open zero-cycle blocks (sext, cat, replicate, mux) must
# run inside an open module scope — in practice, call them from @flow.
#
# NOTE (priority rule, see tc38 / backend UpdatePool): never mix `.default()`
# or a plain bare assign with zero-cycle-gated assigns on the SAME wire — the
# plain assign wins. Full-coverage zif/zelse chains need no default.

from __future__ import annotations

from typing import Sequence, Union

from .. import _session
from ..hw_component import wire, val
from ..signal import SignalRef, to_ref
from ..flow_block import zif, zelif, zelse


def width_of(sig: SignalRef) -> int:
    # Width of a signal *as referenced* — a slice view reports the slice width.
    return to_ref(sig)._slice.size


def zext(sig: SignalRef, width: int) -> SignalRef:
    # Zero-extend to `width` (wraps the native extend op; pass-through if already there).
    sig = to_ref(sig)
    w   = width_of(sig)
    if w == int(width):
        return sig
    if w > int(width):
        raise ValueError(f"zext: signal is {w} bits, wider than target {width}")
    return sig.extend(int(width))


def sext(sig: SignalRef, width: int, name: str | None = None) -> SignalRef:
    # Sign-extend to `width`: low bits pass through, high bits mirror the sign bit.
    sig   = to_ref(sig)
    w     = width_of(sig)
    width = int(width)
    if w == width:
        return sig
    if w > width:
        raise ValueError(f"sext: signal is {w} bits, wider than target {width}")

    out = wire(width, name or _session.auto_name("sext"))
    if w == 1:
        out[0] *= sig
    else:
        out[w - 1, 0] *= sig
    with zif(sig[w - 1] == 1):
        out[width - 1, w] *= val(width - w, -1)
    with zelse():
        out[width - 1, w] *= val(width - w, 0)
    return out


def cat(*sigs: SignalRef, name: str | None = None) -> SignalRef:
    # Concatenate MSB-first (Verilog `{a, b, c}` order) into a fresh wire.
    if not sigs:
        raise ValueError("cat: need at least one signal")
    refs  = [to_ref(s) for s in sigs]
    total = sum(width_of(s) for s in refs)
    out   = wire(total, name or _session.auto_name("cat"))

    pos = 0
    for s in reversed(refs):                    # last argument = least significant
        w = width_of(s)
        if w == 1:
            out[pos] *= s
        else:
            out[pos + w - 1, pos] *= s
        pos += w
    return out


def replicate(bit: SignalRef, n: int, name: str | None = None) -> SignalRef:
    # `n` copies of a 1-bit signal (Verilog `{n{bit}}`).
    bit = to_ref(bit)
    if width_of(bit) != 1:
        raise ValueError("replicate: source must be 1 bit wide")
    return cat(*([bit] * int(n)), name=name or _session.auto_name("repl"))


def _reduce_tree(sigs: Sequence[SignalRef], op) -> SignalRef:
    # Balanced binary fold — log-depth tree instead of a linear chain.
    lvl = [to_ref(s) for s in sigs]
    if not lvl:
        raise ValueError("reduce: need at least one signal")
    while len(lvl) > 1:
        nxt = [op(a, b) for a, b in zip(lvl[0::2], lvl[1::2])]
        if len(lvl) % 2:
            nxt.append(lvl[-1])
        lvl = nxt
    return lvl[0]


def or_reduce(sigs: Sequence[SignalRef]) -> SignalRef:
    # OR together a list of same-width signals (typically 1-bit flags).
    return _reduce_tree(sigs, lambda a, b: a | b)


def and_reduce(sigs: Sequence[SignalRef]) -> SignalRef:
    # AND together a list of same-width signals (typically 1-bit flags).
    return _reduce_tree(sigs, lambda a, b: a & b)


def mux(cond: SignalRef, a: Union[SignalRef, int], b: Union[SignalRef, int],
        name: str | None = None) -> SignalRef:
    # 2:1 mux: `cond ? a : b` as a fresh wire (full-coverage zif/zelse, no default).
    a_ref = a if isinstance(a, int) else to_ref(a)
    b_ref = b if isinstance(b, int) else to_ref(b)
    widths = [width_of(s) for s in (a_ref, b_ref) if isinstance(s, SignalRef)]
    if not widths:
        raise ValueError("mux: at least one of a/b must be a signal (ints have no width)")
    out = wire(max(widths), name or _session.auto_name("mux"))
    with zif(to_ref(cond) == 1):
        out *= a_ref
    with zelse():
        out *= b_ref
    return out


def muxn(sel: SignalRef, items: Sequence[Union[SignalRef, int]],
         name: str | None = None) -> SignalRef:
    # N:1 binary-select mux: `out = items[sel]` as a fresh wire. The LAST item is
    # the zelse arm, so it also covers any sel encoding past len(items)-1 —
    # full coverage, no default needed.
    if not items:
        raise ValueError("muxn: need at least one item")
    refs = [it if isinstance(it, int) else to_ref(it) for it in items]
    if len(refs) == 1:
        ref = refs[0]
        if isinstance(ref, int):
            raise ValueError("muxn: a single item must be a signal (ints have no width)")
        return ref
    widths = [width_of(s) for s in refs if isinstance(s, SignalRef)]
    if not widths:
        raise ValueError("muxn: at least one item must be a signal (ints have no width)")
    sel = to_ref(sel)
    out = wire(max(widths), name or _session.auto_name("muxn"))
    with zif(sel == 0):
        out *= refs[0]
    for i in range(1, len(refs) - 1):
        with zelif(sel == i):
            out *= refs[i]
    with zelse():
        out *= refs[-1]
    return out


def decoder(sel: SignalRef, n: int) -> list[SignalRef]:
    # Binary → one-hot: n 1-bit expressions, entry i high when sel == i.
    sel = to_ref(sel)
    if n < 1:
        raise ValueError("decoder: n must be >= 1")
    return [sel == i for i in range(int(n))]


def priority_encoder(flags: Sequence[SignalRef],
                     name: str | None = None) -> tuple[SignalRef, SignalRef]:
    # Lowest-set-index encoder over 1-bit flags. Returns (index_wire, valid):
    # index of the first high flag (0 when none), valid = OR of all flags.
    refs = [to_ref(f) for f in flags]
    if not refs:
        raise ValueError("priority_encoder: need at least one flag")
    iw  = max(1, (len(refs) - 1).bit_length())
    idx = wire(iw, name or _session.auto_name("prienc"))
    with zif(refs[0] == 1):
        idx *= 0
    for i in range(1, len(refs)):
        with zelif(refs[i] == 1):
            idx *= i
    with zelse():
        idx *= 0
    return idx, or_reduce(refs)
