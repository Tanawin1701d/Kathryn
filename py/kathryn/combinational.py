# Combinational combinators — a select (`mux`), a rotation (`rotate_left`),
# an OR reduce (`any_of`) and a population count / adder tree (`sum_cnt`).
#
# THE LOGIC LIVES IN THE RUST CORE (`src/model/arena_impl_comb.rs`), not here:
# the topology (wire + zif/zelse mux, balanced trees), the width rules and the
# validations are frontend-agnostic, so they sit on `ModelArena` where any
# frontend gets the same hardware from the same rules. This module is the thin
# Python face: it resolves refs, auto-names, and forwards int literals for the
# connector to wrap (`arena_impl_comb_py.rs`). See the core file for the design
# rationale (why mux is a zif/zelse pair, why the trees are balanced, why
# sum_cnt's default width cannot overflow, ...).
#
# What each returns:
# - `mux` DECLARES hardware (a wire plus a zif/zelse pair), so it must be
#   called inside an open flow scope, and it composes:
#   `mux(c1, x, mux(c2, y, z))`. Inside a `seq()` the emitted always-block is
#   gated on that step's state — read it in the step that built it.
# - `rotate_left`, `any_of`, `sum_cnt` are pure expressions, legal wherever an
#   expression is. A zero rotate / single-term `any_of` returns the input
#   signal unchanged (slice view intact) — the core signals identity with None.

from __future__ import annotations

from typing import Optional, Sequence, Union

from . import _session
from .signal import SignalRef, expr, to_ref

Operand = Union[SignalRef, int]


def _operand(x: Operand) -> tuple:
    # An int passes through raw — the Rust connector wraps it into a val sized
    # to the mux width. A signal resolves to its handle + read slice.
    if isinstance(x, int):
        return x, None
    ref = to_ref(x)
    return ref._ident, ref._slice


# ---- select -----------------------------------------------------------------
def mux(cond      : SignalRef,
        if_true   : Operand,
        if_false  : Operand,
        width     : Optional[int] = None,
        name      : Optional[str] = None) -> SignalRef:
    """`cond ? if_true : if_false` as a combinational wire.

    Declares hardware, so it must be called inside an open flow scope. `width`
    defaults to the width of the first operand that is a signal; give it
    explicitly when both arms are int literals, or when the result is wider
    than its arms.

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
def rotate_left(signal : SignalRef,
                amount : int = 1,
                width  : Optional[int] = None) -> SignalRef:
    """`signal` rotated left by `amount`, as a pure expression.

    `width` defaults to the signal's own width and is worth passing only to
    rotate WITHIN a narrower field of a wider signal; it may not exceed the
    signal. `amount` is an elaboration-time constant taken mod `width`, so a
    full turn is the identity and returns the signal unchanged.

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
def any_of(terms  : Sequence[SignalRef],
           name   : Optional[str] = None) -> SignalRef:
    """True when any of `terms` is — a balanced OR tree over 1-bit signals.

    Pure expression. No terms is false, not an error: unlike a sum, an empty
    disjunction has a defined answer and a defined width.

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
def sum_cnt(bits  : Sequence[SignalRef],
            width : Optional[int] = None,
            name  : Optional[str] = None) -> SignalRef:
    """How many of `bits` are set — a balanced adder tree over 1-bit signals.

    Pure expression: builds no wire and opens no flow block, so unlike `mux` it
    is legal wherever an expression is. The default `width` cannot overflow.

        freed = sum_cnt([e.valid for e in commit_entries])
    """
    refs = [to_ref(bit) for bit in bits]
    out  = _session.arena().gen_sum_cnt(
        name or _session.auto_name("expr"),
        [(ref._ident, ref._slice) for ref in refs], width)
    return expr(out)
