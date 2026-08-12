# Karray (typed multi-dimensional array CCP) DSL surface. Both handles are kept
# MINIMAL — like a signal handle, they store only Rust idents and route everything
# back through the singleton arena (which owns all layout: shape, fields, widths,
# backing). `Karray` holds just the CcpIdent; `KarrayRef` (karray_ref.py) holds
# the CcpIdent plus the accumulated per-dimension keys and an optional field name.
#
# Indexing yields a `KarrayRef` (`d[2][1]`, `d[0, 2]`, `d[sig]`, `d[fn]` — the
# four unified index kinds, see karray_ref.py) and a field is selected by
# attribute (`d[2][1].valid`). Each field is its own HCP: a field ref drives
# that field's hardware; a whole-element assign (`d[2][1] |= {"valid": a,
# "data": b}`) connects each named source to the field of that name (a bare
# scalar source is allowed on single-field Karrays); a karray-to-karray assign
# (`d[0, 1] |= e[2, 3]`) pairs kept dims 1:1 and fields by name+width — all
# inside Rust. Assignment always uses `|=` (reg backing) or `*=` (wire backing);
# a bare `=` is rejected.

from __future__ import annotations

from typing import Iterable, Optional, Union

from .. import _session
from ..signal import _ASSIGNED
from .karray_field import (
    collect_declared_karray_fields,
    get_declared_karray_fields,
    normalize_karray_field_specs,
)
from .karray_ref import KarrayRef

__all__ = ["Karray"]


# ---- karray -----------------------------------------------------------------
class Karray:
    """Typed multi-dimensional array (Karray CCP) — a thin handle over the Rust
    CcpIdent (no cached layout). Declare element fields with `kaf()` on a subclass;
    the constructor is `(backing, shape=(1,), name=None)` where `backing` is a
    `kathryn.HwComponentType` member — `REG` (clocked, `|=`) or `WIRE`
    (combinational, `*=`)."""

    __slots__         = ("_ident",)
    __karray_fields__ = ()

    def __init_subclass__(cls, **kwargs) -> None:
        super().__init_subclass__(**kwargs)
        # Shared walk with KBundle: inherited flat fields first, then cls's own
        # kaf() specs, nested bundles flattened with an underscore prefix.
        cls.__karray_fields__ = collect_declared_karray_fields(cls)

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

    # Whole-array karray-to-karray assignment (`dst |= src`). The subscript lands
    # on a plain name, so return `self` to keep the variable bound (subscripted
    # forms return the _ASSIGNED sentinel instead — see KarrayRef).
    def __ior__(self, src: Union["Karray", KarrayRef]):
        KarrayRef(self._ident)._assign_from(src, expect_clocked=True)
        return self

    def __imul__(self, src: Union["Karray", KarrayRef]):
        KarrayRef(self._ident)._assign_from(src, expect_clocked=False)
        return self

    def __setitem__(self, key, value) -> None:
        # Tail of `d[i] |= x`, where the subscript lands directly on the Karray
        # (not a KarrayRef): the KarrayRef already did the assign and returned
        # the sentinel. A real value is a bare `=`, which is rejected.
        if value is _ASSIGNED:
            return
        self[key]._assign_from(value, expect_clocked=None)
