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

from typing import Iterable, Optional, Tuple, Union

from .. import _session
from .._kathryn import HwComponentType
from ..signal import _ASSIGNED
from .karray_ref import KarrayRef


# ---- karray -----------------------------------------------------------------
class Karray:
    """Typed multi-dimensional array (Karray CCP) — a thin handle over the Rust
    CcpIdent (no cached layout). `shape` is the dimension tuple (e.g. `(5, 3)`);
    `fields` is the element record as `(name, width)` pairs, in declaration order,
    each materialised as its OWN hardware component. `backing` is a
    `kathryn.HwComponentType` member — `REG` / `WIRE` / `MEM_BLOCK`."""

    __slots__ = ("_ident",)

    def __init__(self,
                 shape   : Iterable[int],
                 fields  : Iterable[Tuple[str, int]],
                 backing : int = HwComponentType.REG,
                 name    : Optional[str] = None) -> None:
        name        = name or _session.auto_name("karray")
        flds        = [(str(n), int(w)) for (n, w) in fields]
        self._ident = _session.arena().mk_karray(name, [int(d) for d in shape], flds, int(backing))

    @property
    def ident(self):                          # the underlying CcpIdent
        return self._ident

    def __getitem__(self, key) -> KarrayRef:
        return KarrayRef(self._ident)[key]

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
