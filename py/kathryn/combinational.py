# Combinational combinators built on the primitives — a select (`mux`) and a
# population count / adder tree (`sum_cnt`). Neither adds a Rust node type: each
# is a small assembly of what signal.py, hw_component.py and flow_block.py
# already provide, kept here so every caller writes the same one.
#
# This module sits ABOVE signal/hw_component/flow_block in the import order
# (`mux` needs a wire and a zif/zelse pair), which is why it is not part of
# signal.py.
#
# Decisions:
# - `mux` is a WIRE plus `zif`/`zelse`, not a mask expression. Kathryn has no
#   ternary LogicOp, so the expression-only routes are the two-operand tricks:
#   `(a & m) | (b & ~m)` needs a bit-replicated mask, which `extend` cannot make
#   (mk_extend_bit fills with 1'b0 — zero-extension, not replication), and
#   `0 - sel` leans on the emitted expression's wrap width. The zif/zelse chain
#   is the tested priority-mux path and emits a plain `if/else`.
#   COST, and it is real: `mux` DECLARES hardware, so it must be called inside
#   an open flow scope, and it is a statement that yields a value rather than a
#   pure expression. Calling it in `@init` puts the wire in the module but
#   leaves the branches with nowhere to attach.
# - `mux` returns the wire, so it composes: `mux(c1, x, mux(c2, y, z))`. The
#   inner call runs before the outer `zif` opens, so its wire lands in the same
#   enclosing scope, not nested inside a branch.
# - A mux inherits the SCOPE it was opened in, which matters inside `seq()`: the
#   emitted always-block is gated on that sequential step's state, so the wire
#   reads its default (0) on every other step. Read it in the same step that
#   built it; a value some later step needs belongs in a reg.
# - `sum_cnt` is a BALANCED tree, not a left-to-right chain: log2(n) adder
#   depth instead of n, which is what decides fmax once a machine commits 4+
#   entries a cycle. The odd element of a level rides to the next level
#   unchanged rather than forcing a ragged extra stage.
# - `sum_cnt`'s result width is derived to be exactly wide enough for the
#   largest sum the inputs can make, so it can never overflow. With the 1-bit
#   inputs it is meant for that is `len(bits).bit_length()` — 4 bits count 0..4
#   in 3 — and the same formula stays correct if someone feeds it wider values.
# - Both take an optional `width`: an explicit width is a statement, and a
#   caller sizing a record field from a description already knows the number.

from __future__ import annotations

from typing import Optional, Sequence, Union

from . import _session
from .flow_block import zelse, zif
from .hw_component import wire
from .signal import SignalRef, to_ref

Operand = Union[SignalRef, int]


# ---- width helpers ----------------------------------------------------------
def _width_of(ref: SignalRef) -> int:
    # The width a ref actually reads: a slice view is narrower than its owner.
    return ref._slice.stop - ref._slice.start


def _operand_width(*operands: Operand) -> int:
    # Width of the first real signal among the operands — an int has none of its
    # own (the connector sizes a literal from whatever it meets).
    for operand in operands:
        if isinstance(operand, SignalRef):
            return _width_of(operand)
    raise TypeError(
        "cannot infer a width from int operands alone — pass width=<bits> "
        "(an int literal is sized by whatever it is used with)")


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
    out = wire(width if width is not None else _operand_width(if_true, if_false),
               name or _session.auto_name("mux"))
    # Declared BEFORE the branches open, so the wire belongs to the enclosing
    # scope and only its two assignments live inside the arms.
    with zif(to_ref(cond)):
        out *= if_true
    with zelse():
        out *= if_false
    return out


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
    if not refs:
        raise ValueError("sum_cnt of no signals has no width — pass at least one")

    # Widest a sum of these can get: len(refs) copies of the largest input.
    max_in  = max(_width_of(ref) for ref in refs)
    natural = (len(refs) * ((1 << max_in) - 1)).bit_length()
    out_w   = width if width is not None else natural

    # Every term starts at the result width, so no intermediate add truncates.
    level = [ref.extend(out_w) for ref in refs]
    while len(level) > 1:
        paired = [level[i] + level[i + 1] for i in range(0, len(level) - 1, 2)]
        if len(level) % 2:
            paired.append(level[-1])        # odd one out rides to the next level
        level = paired
    return level[0]
