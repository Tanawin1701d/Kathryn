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

from itertools import product
from typing import Iterable, Optional, Union

from .. import _session
from ..signal import SignalRef, _ASSIGNED
from .karray_field import (
    RESERVED_FIELD_NAMES,
    collect_declared_karray_fields,
    get_declared_karray_fields,
    resolve_karray_field_specs,
)
from .karray_ref import KarrayRef

__all__ = ["Karray"]


# ---- karray -----------------------------------------------------------------
class Karray:
    """Typed multi-dimensional array (Karray CCP) — a thin handle over the Rust
    CcpIdent (no cached layout). Declare element fields with `kaf()` on a subclass;
    the constructor is `(backing, shape=(1,), name=None, **fields)` where `backing`
    is a `kathryn.HwComponentType` member — `REG` (clocked, `|=`) or `WIRE`
    (combinational, `*=`).

    The class body states the shape a record usually has; the call finishes it.
    A keyword's VALUE picks what it does — an int sets the width of a DECLARED
    field, a `kaf()` ADDS a field this array has and the class does not:

        class Entry(Karray):
            pc    = kaf(32)        # 32 by default
            instr = kaf()          # no default: every instantiation says

        Entry(HwComponentType.REG, (lanes,), "fetch",
              pc=64, instr=16,     # widths for what the class declares
              spectag=kaf(8))      # a field only this array carries

    Added fields land after the declared ones in keyword order, flatten like any
    bundle (`tag=kaf(Vec2)` -> "tag_x", "tag_y"), and read back through the same
    attribute chain (`d[0].spectag`). An int naming no declared field raises, so
    a typo cannot silently do nothing, and a `kaf()` naming one that IS declared
    raises rather than shadowing it. The class's own field list is never
    mutated."""

    __slots__         = ("_ident",)
    __karray_fields__ = ()

    def __init_subclass__(cls, **kwargs) -> None:
        super().__init_subclass__(**kwargs)
        # Shared walk with KBundle: inherited flat fields first, then cls's own
        # kaf() specs, nested bundles flattened with an underscore prefix.
        cls.__karray_fields__ = collect_declared_karray_fields(cls)
        # Field arguments ride in as keywords, so a field may not be named after
        # one of __init__'s own parameters. Caught here, when the class is
        # written, rather than at a confusing call site later.
        clash = [name for name, _ in cls.__karray_fields__
                 if name in RESERVED_FIELD_NAMES]
        if clash:
            raise TypeError(
                f"{cls.__name__}: field name(s) {', '.join(clash)} collide with "
                f"Karray.__init__ parameters ({', '.join(RESERVED_FIELD_NAMES)}) "
                f"— rename the field, or give it another name with kaf(w, 'other')")

    def __init__(self,
                 backing : int,
                 shape   : Iterable[int] = (1,),
                 name    : Optional[str] = None,
                 **fields) -> None:
        name        = name or _session.auto_name("karray")
        flds        = resolve_karray_field_specs(
            get_declared_karray_fields(type(self)), fields,
            f"{type(self).__name__} '{name}'")
        self._ident = _session.arena().mk_karray(name, [int(d) for d in shape], flds, int(backing))

    @property
    def ident(self):                          # the underlying CcpIdent
        return self._ident

    # ---- reset ---------------------------------------------------------------
    def reset(self, **fields) -> "Karray":
        """Reset value for EVERY element of a reg-backed Karray, one keyword per
        field: `rat.reset(renamed=0, prf_idx=0)`.

        It records the value on each element's own backing register and calls
        `reg.reset`, so the reset event, its priority (DEFAULT_UE_PRI_RST) and its
        clock are the register's own — a Karray adds NO reset mechanism of its
        own, it only says which registers to point the existing one at. A field
        left out of the call keeps no reset value and powers up undefined, the
        same as a bare `reg`.

        Whole-array only: one value per field, shared by every element. That is
        what a state array wants (a rename table's valid bits all reset to 0);
        a per-element reset would need a static element handle, which is a
        different feature and is left until something needs it."""
        arena = _session.arena()
        if not arena.karray_is_clocked(self._ident):
            raise TypeError("reset(...) requires a reg-backed Karray "
                            "(a wire has no state to reset; use default(...))")
        coords = [list(c) for c in product(*(range(d) for d in arena.karray_shape(self._ident)))]
        for name, value in fields.items():
            # Coerce ONCE: every element of a field has the same width, so one
            # val backs all of them rather than one val per element.
            value = SignalRef(
                arena.karray_element_hcp(self._ident, coords[0], name))._coerce_value(value)
            for coord in coords:
                SignalRef(arena.karray_element_hcp(self._ident, coord, name)).reset(value)
        return self

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
