# Combinational combinators: `mux`, `rotate_left`, `any_of`, `sum_cnt`.
#
# THE LOGIC IS IN THE RUST CORE (src/model/arena_impl_comb.rs), NOT here —
# topology, width rules and validation are frontend-agnostic, so every frontend
# builds the same hardware. This file only resolves refs, auto-names, and hands
# int literals to the connector (arena_impl_comb_py.rs) to wrap.
#
# - `mux` DECLARES hardware (a wire + a zif/zelse pair) -> needs an OPEN FLOW
#   SCOPE. It composes: `mux(c1, x, mux(c2, y, z))`.
# - Inside a `seq()` a mux's always-block is gated on THAT STEP's state, so read
#   it in the step that built it; a value a later step needs belongs in a reg.
# - `rotate_left` / `any_of` / `sum_cnt` are pure expressions — legal anywhere.
# - Identity cases (full-turn rotate, single-term `any_of`) come back from the
#   core as None; the input ref is returned unchanged, slice view intact.

from __future__ import annotations

from typing import Optional, Sequence, Tuple, Union

from . import _session
from ._kathryn import HcpIdent, Slice
from .signal import Operand, SignalRef, Source, expr, to_ref

# What the connector takes per operand: a handle + its read slice, or a raw int
# it will size against the other side.
ConnOperand = Tuple[Union[int, HcpIdent], Optional[Slice]]


def _operand(x: Operand) -> ConnOperand:
    # An int passes through raw — the Rust connector wraps it into a val sized
    # to the mux width. A signal resolves to its handle + read slice.
    if isinstance(x, int):
        return x, None
    ref = to_ref(x)
    return ref._ident, ref._slice


# ---- select -----------------------------------------------------------------
def mux(cond      : Source,
        if_true   : Operand,
        if_false  : Operand,
        width     : Optional[int] = None,
        name      : Optional[str] = None) -> SignalRef:
    """`cond ? if_true : if_false` as a combinational wire.

    - DECLARES hardware — call it inside an open flow scope.
    - `width` defaults to the first arm that is a SIGNAL; state it when both
      arms are int literals, or when the result is wider than its arms.

        self.next_pc *= mux(taken, target, self.pc + 4)
    """
    cond_ref     = to_ref(cond)
    t_i, t_slice = _operand(if_true)
    f_i, f_slice = _operand(if_false)
    out = _session.arena().gen_mux(
        name or _session.auto_name("mux"), cond_ref._ident, cond_ref._slice,
        t_i, f_i, t_slice, f_slice, width)
    return SignalRef(out)


# ---- rotation ---------------------------------------------------------------
def rotate_left(signal : Source,
                amount : int = 1,
                width  : Optional[int] = None) -> SignalRef:
    """`signal` rotated left by `amount`, as a pure expression.

    - `width` defaults to the signal's own and may NOT exceed it; pass it only
      to rotate WITHIN a narrower field of a wider signal.
    - `amount` is an elaboration-time constant taken mod `width`, so a full turn
      is the identity and gives the signal back unchanged.

        self.next_tag |= rotate_left(self.next_tag)     # one-hot tag, step one
    """
    ref = to_ref(signal)
    # Python-type guards only — the numeric rules (width >= 1, width fits the
    # signal, amount mod width) are the core's.
    if width is not None and (isinstance(width, bool) or not isinstance(width, int)):
        raise ValueError(f"rotate_left needs a width >= 1, got {width!r}")
    if isinstance(amount, bool) or not isinstance(amount, int):
        raise TypeError(
            f"rotate_left amount must be an int, got {type(amount).__name__}")

    out = _session.arena().gen_rotate_left(
        _session.auto_name("rol"), ref._ident, ref._slice, amount, width)
    return ref if out is None else expr(out)    # None = full turn: identity


# ---- reduction ---------------------------------------------------------------
def any_of(terms  : Sequence[Source],
           name   : Optional[str] = None) -> SignalRef:
    """True when any of `terms` is — a balanced OR tree over 1-bit signals.

    - Pure expression; an OR tree beats `sum_cnt(...) != 0` for "is any set".
    - NO terms is FALSE, not an error: unlike a sum, an empty disjunction has a
      defined answer and a defined width.

        self.over_use *= any_of([(free == 0).land(r) for r in self.req_port])
    """
    refs = [to_ref(term) for term in terms]
    out  = _session.arena().gen_any_of(
        name or _session.auto_name("none" if not refs else "expr"),
        [(ref._ident, ref._slice) for ref in refs])
    if out is None:                             # single term: identity
        return refs[0]
    return expr(out) if refs else SignalRef(out)   # empty -> the constant-0 val


# ---- population count / adder tree ------------------------------------------
def sum_cnt(bits  : Sequence[Source],
            width : Optional[int] = None,
            name  : Optional[str] = None) -> SignalRef:
    """How many of `bits` are set — a balanced adder tree over 1-bit signals.

    - Pure expression: no wire, no flow block, so legal wherever an expression
      is (unlike `mux`).
    - The default `width` is derived so the sum CANNOT overflow.

        freed = sum_cnt([e.valid for e in commit_entries])
    """
    refs = [to_ref(bit) for bit in bits]
    out  = _session.arena().gen_sum_cnt(
        name or _session.auto_name("expr"),
        [(ref._ident, ref._slice) for ref in refs], width)
    return expr(out)
