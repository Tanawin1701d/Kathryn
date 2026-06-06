# Signal handles + operator overloading. A SignalRef carries ONLY the Rust
# HcpIdent (plus an optional bit-slice); every operation routes back through the
# singleton arena, so Rust remains the sole owner of all model objects.

from . import _session
from ._kathryn import LogicOp, Slice


# Sentinel returned by a *sliced* augmented-assign (`a[hi,lo] |= x`) so the
# implicit __setitem__ that Python emits can recognise the work is already done.
class _Assigned:
    __slots__ = ()


_ASSIGNED = _Assigned()


def to_ref(x):
    # Accept any SignalRef (signal, slice view, or expression result).
    if isinstance(x, SignalRef):
        return x
    raise TypeError(f"expected a kathryn signal, got {type(x).__name__}: {x!r} "
                    f"(wrap constants with val(width, n))")


def _inclusive_slice(key):
    # `sig[hi, lo]` -> bits hi..lo inclusive; `sig[i]` -> single bit i.
    # Maps to the half-open Rust Slice(start=lo, stop=hi+1).
    if isinstance(key, tuple):
        if len(key) != 2:
            raise TypeError("slice index must be sig[hi, lo] or sig[i]")
        hi, lo = key
    elif isinstance(key, slice):
        raise TypeError("use inclusive comma form sig[hi, lo], not sig[hi:lo]")
    else:
        hi = lo = key
    return Slice(int(lo), int(hi) + 1)


class SignalRef:
    """A signal handle plus optional bit-slice. Holds only the Rust HcpIdent."""

    __slots__ = ("_ident", "_slice")

    def __init__(self, ident, slc=None):
        self._ident = ident
        self._slice = slc

    # Assignability is a property of the underlying component, read straight off
    # the Rust ident: True = reg/mem (assign with |=), False = wire/io_wire
    # (assign with *=), None = not an assignment destination (val, expr result).
    @property
    def _clocked(self): return self._ident.clocked

    # ---- identity ----------------------------------------------------------
    @property
    def ident    (self): return self._ident
    @property
    def global_id(self): return self._ident.global_id
    @property
    def hw_type  (self): return self._ident.hw_type

    def __repr__(self):
        sl = "" if self._slice is None else f"[{self._slice.start},{self._slice.stop})"
        return f"{type(self).__name__}({self._ident!r}{sl})"

    # ---- inclusive slicing -------------------------------------------------
    def __getitem__(self, key):
        # Slice view shares the same ident, so it inherits the owner's clocked-ness.
        return SignalRef(self._ident, _inclusive_slice(key))

    # ---- expression building ----------------------------------------------
    def _binop(self, other, op): # two op
        other = to_ref(other)
        out = _session.arena().mk_expression(
            _session.auto_name("expr"), int(op),
            self._ident, other._ident, self._slice, other._slice,
        )
        return expr(out)

    def _unop(self, op): # uni-op
        out = _session.arena().mk_expression_single(
            _session.auto_name("expr"), int(op), self._ident, self._slice,
        )
        return expr(out)

    # arithmetic
    def __add__     (self, o): return self._binop(o, LogicOp.ArithPlus)
    def __sub__     (self, o): return self._binop(o, LogicOp.ArithMinus)
    def __mul__     (self, o): return self._binop(o, LogicOp.ArithMul)
    def __truediv__ (self, o): return self._binop(o, LogicOp.ArithDiv)
    def __mod__     (self, o): return self._binop(o, LogicOp.ArithDivr)
    # bitwise
    def __and__     (self, o): return self._binop(o, LogicOp.BitwiseAnd)
    def __or__      (self, o): return self._binop(o, LogicOp.BitwiseOr)
    def __xor__     (self, o): return self._binop(o, LogicOp.BitwiseXor)
    def __lshift__  (self, o): return self._binop(o, LogicOp.BitwiseShl)
    def __rshift__  (self, o): return self._binop(o, LogicOp.BitwiseShr)
    def __invert__  (self):    return self._unop(LogicOp.BitwiseInvr)
    # relational (NOTE: == / != return an expr, not a bool; see __hash__)
    def __eq__      (self, o): return self._binop(o, LogicOp.RelationEq)
    def __ne__      (self, o): return self._binop(o, LogicOp.RelationNeq)
    def __lt__      (self, o): return self._binop(o, LogicOp.RelationLe)
    def __le__      (self, o): return self._binop(o, LogicOp.RelationLeq)
    def __gt__      (self, o): return self._binop(o, LogicOp.RelationGe)
    def __ge__      (self, o): return self._binop(o, LogicOp.RelationGeq)

    # Ops with no Python operator: logical &&/||/!, signed compares, bit-extend.
    def land  (self, o): return self._binop(o, LogicOp.LogicalAnd)
    def lor   (self, o): return self._binop(o, LogicOp.LogicalOr)
    def lnot  (self):    return self._unop(LogicOp.LogicalNot)
    def slt   (self, o): return self._binop(o, LogicOp.RelationSlt)
    def sgt   (self, o): return self._binop(o, LogicOp.RelationSgt)
    def extend(self, width):
        out = _session.arena().mk_extend_bit(
            _session.auto_name("expr"), self._ident, int(width), self._slice)
        return expr(out)

    # == returns an expr, so SignalRef cannot be a usable hash key; use `is`.
    __hash__ = None

    # ---- assignment --------------------------------------------------------
    def _do_assign(self, src):
        src = to_ref(src)
        _session.arena().gen_basic_assign(
            self._ident, src._ident, self._slice, src._slice)
        # Whole-signal `a |= x` rebinds name a -> return self (no-op rebind).
        # Sliced `a[h,l] |= x` is desugared to a __setitem__ -> return sentinel.
        return _ASSIGNED if self._slice is not None else self

    def __ior__(self, src):
        if self._clocked is not True:
            raise TypeError("`|=` (clocked assign) requires a reg / mem_blk / mem_ele destination")
        return self._do_assign(src)

    def __imul__(self, src):
        if self._clocked is not False:
            raise TypeError("`*=` (combinational assign) requires a wire / io_wire destination")
        return self._do_assign(src)

    def __setitem__(self, key, value):
        # Implicit tail of `a[h,l] |= x`: the slice view already did the assign.
        if value is _ASSIGNED:
            return
        # Explicit `a[h,l] = rhs`: assign with the destination slice.
        if self._clocked is None:
            raise TypeError(f"{type(self).__name__} is not an assignment destination")
        rhs = to_ref(value)
        _session.arena().gen_basic_assign(
            self._ident, rhs._ident, _inclusive_slice(key), rhs._slice)


class expr(SignalRef):
    """Result of an operator/expression. Not user-constructed; not assignable."""
    __slots__ = ()
