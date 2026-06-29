# Karray (typed multi-dimensional array CCP) DSL surface. Both handles are kept
# MINIMAL — like a signal handle, they store only Rust idents and route everything
# back through the singleton arena (which owns all layout: shape, fields, widths,
# backing). `Karray` holds just the CcpIdent; `KarrayRef` holds the CcpIdent plus
# the accumulated index/slice keys and an optional field name. Indexing yields a
# `KarrayRef` (`d[2][1]`, `d[0:2]`) and a field is selected by attribute
# (`d[2][1].valid`). Each field is its own HCP: a field ref drives that field's
# hardware; a whole-element assign (`d[2][1] |= {"valid": a, "data": b}`) connects
# each named source to the field of that name (full-width, no bit-level split); a
# karray-to-karray assign (`d[0:2] |= e[1:3]`) pairs fields by name+width across
# equal-shaped regions (also inside Rust).

from __future__ import annotations

from typing import Iterable, Optional, Union

from .. import _session
from .._kathryn import HwComponentType
from ..signal import _ASSIGNED, SignalRef, to_ref
from .karray_field import (
    KarrayField,
    get_declared_karray_fields,
    normalize_karray_field_specs,
)
from .karray_ref import KarrayRef


# ---- reduce: fold marker + the view handed to the user select function -------
class _ReduceMarker:
    """Sentinel for a Karray dimension that should be reduced (folded), as opposed
    to an int that pins it. Use the singleton `Reduce`: `k.reduce([Reduce, 2], fn)`."""
    __slots__ = ()
    def __repr__(self) -> str:
        return "Reduce"


Reduce = _ReduceMarker()


class ReduceView:
    """One subtree handed to a reduce select function. `.fields` maps each carried
    field name to its current SignalRef (a leaf element's field, or a mux-output
    wire); `.indices` is the list of element coordinates this subtree covers."""
    __slots__ = ("indices", "fields")

    def __init__(self, indices, fields) -> None:
        self.indices = indices    # list of [coord] lists
        self.fields  = fields     # dict: field name -> SignalRef


# ---- karray -----------------------------------------------------------------
class Karray:
    """Typed multi-dimensional array (Karray CCP) — a thin handle over the Rust
    CcpIdent (no cached layout). Declare element fields with `kaf()` on a subclass;
    the constructor is `(backing, shape=(1,), name=None)` where `backing` is a
    `kathryn.HwComponentType` member — `REG` / `WIRE` / `MEM_BLOCK`."""

    __slots__         = ("_ident",)
    __karray_fields__ = ()

    def __init_subclass__(cls, **kwargs) -> None:
        super().__init_subclass__(**kwargs)

        # Collect inherited fields first (MRO base→parent order), then cls's own.
        fields = []
        seen   = set()
        for base in reversed(cls.__mro__[1:]):
            for name, width in get_declared_karray_fields(base):
                if name not in seen:
                    fields.append((name, width))
                    seen.add(name)

        for _, spec in cls.__dict__.items():
            if not isinstance(spec, KarrayField):
                continue
            if spec.name in seen:
                raise TypeError(f"duplicate Karray field name: {spec.name}")
            fields.append((spec.name, spec.width))
            seen.add(spec.name)

        cls.__karray_fields__ = tuple(fields)

    def __init__(self,
                 backing : int,
                 shape   : Iterable[int] = (1,),
                 name    : Optional[str] = None) -> None:
        name        = name or _session.auto_name("karray")
        flds        = normalize_karray_field_specs(get_declared_karray_fields(type(self)))
        self._ident = _session.arena().mk_karray(name, [int(d) for d in shape], flds, int(backing))

    @property
    def ident(self):                          # the underlying CcpIdent
        return self._ident

    def __getitem__(self, key) -> KarrayRef:
        return KarrayRef(self._ident)[key]

    # ---- generic callback-driven reduce ------------------------------------
    def reduce(self, dims, select_fn, fields=None) -> KarrayRef:
        """Reduce elements to a single winner via a user select function. `dims` has
        one entry per dimension: an int pins it, `Reduce` folds (reduces) it. For each
        compared pair the tree calls `select_fn(a, b, level)` — `a`/`b` are
        `ReduceView`s (`.fields`, `.indices`), `level` is the tree depth — which must
        return a 1-bit signal that is true to pick `a` (left). `fields` limits which
        fields are carried (default: all). Returns the winner as a scalar KarrayRef,
        so read it as `winner.field`. Reg/Wire backings only."""
        enc = []
        for d in dims:
            if d is Reduce:
                enc.append(None)
            elif isinstance(d, int):
                enc.append(int(d))
            else:
                raise TypeError("reduce dim must be an int (pin) or Reduce (fold)")

        flds = list(fields) if fields is not None else [name for (name, _w) in type(self).__karray_fields__]

        # raw callback the Rust loop calls; wraps the user fn with friendly views.
        def _raw(a_fields, a_indices, b_fields, b_indices, level):
            a = ReduceView(a_indices, {n: SignalRef(h) for (n, h) in a_fields})
            b = ReduceView(b_indices, {n: SignalRef(h) for (n, h) in b_fields})
            return to_ref(select_fn(a, b, level))._ident   # 1-bit select-left HcpIdent

        res_ccp = _session.arena().karray_reduce(self._ident, enc, flds, _raw)
        return KarrayRef(res_ccp, [0])   # scalar winner -> read fields as winner.field

    # Whole-array karray-to-karray assignment (`dst |= src`). The subscript lands on
    # a plain name, so return `self` to keep the variable bound (subscripted forms
    # return the _ASSIGNED sentinel instead — see KarrayRef).
    def __ior__(self, src: Union["Karray", KarrayRef]):
        KarrayRef(self._ident)._assign_from(src, expect_clocked=True)
        return self

    def __imul__(self, src: Union["Karray", KarrayRef]):
        KarrayRef(self._ident)._assign_from(src, expect_clocked=False)
        return self

    def __setitem__(self, key, value) -> None:
        # Tail of `d[i] |= x` / `d[0:2] |= x`, where the subscript lands directly on
        # the Karray (not a KarrayRef): the KarrayRef already did the assign and
        # returned the sentinel. A real value is an explicit `d[i] = rhs`.
        if value is _ASSIGNED:
            return
        self[key]._explicit_assign(value)
