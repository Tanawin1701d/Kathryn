# Karray (typed multi-dimensional array CCP) DSL surface. Both handles are kept
# MINIMAL — like a signal handle, they store only Rust idents and route everything
# back through the singleton arena (which owns all layout: shape, fields, widths,
# backing). `Karray` holds just the CcpIdent; `KarrayRef` holds the CcpIdent plus
# the accumulated indices and an optional field name. Indexing yields a `KarrayRef`
# (`d[2][1]`) and a field is selected by attribute (`d[2][1].valid`). Each field is
# its own HCP: a field ref drives that field's hardware; a whole-element assign
# (`d[2][1] |= packed`) is split across the per-field HCPs inside Rust.

from __future__ import annotations

from typing import Iterable, Optional, Sequence, Tuple, Union

from .. import _session
from .._kathryn import HwComponentType
from ..signal import SignalRef, _ASSIGNED, to_ref


# ---- karray element / field reference ---------------------------------------
class KarrayRef:
    """A partially- or fully-indexed view into a Karray, optionally narrowed to a
    field. Holds only the Karray's CcpIdent plus the accumulated indices and field
    name; shape/field validation happens in Rust at resolution time."""

    __slots__ = ("_ident", "_indices", "_field")

    def __init__(self, ident, indices: Optional[Sequence[int]] = None, field: Optional[str] = None) -> None:
        # Bypass our own __setattr__ (reserved for the assignment tail).
        object.__setattr__(self, "_ident",   ident)
        object.__setattr__(self, "_indices", list(indices) if indices else [])
        object.__setattr__(self, "_field",   field)

    # ---- indexing / field selection ----------------------------------------
    def __getitem__(self, key: int) -> "KarrayRef":
        if self._field is not None:
            raise TypeError("cannot index into a Karray field")
        if not isinstance(key, int):
            raise TypeError("Karray index must be a static int (dynamic indices are not supported yet)")
        return KarrayRef(self._ident, self._indices + [key], None)

    def __getattr__(self, name: str) -> "KarrayRef":
        # Only reached when normal (slot) lookup fails, so this is a field name.
        # The name is validated in Rust when the ref is resolved (see karray_field_hcp).
        if name.startswith("_"):
            raise AttributeError(name)
        return KarrayRef(self._ident, self._indices, name)

    # ---- resolution --------------------------------------------------------
    def _field_hcp(self, is_read: bool):
        return _session.arena().karray_field_hcp(self._ident, list(self._indices), self._field, bool(is_read))

    # Read hook consumed by signal.to_ref (use a field as an assignment source).
    def _to_read_ref(self) -> SignalRef:
        if self._field is None:
            raise TypeError("read a specific field (d[i][j].field), not a whole Karray element")
        return SignalRef(self._field_hcp(is_read=True))

    # ---- assignment --------------------------------------------------------
    def _assign(self, src: SignalRef, expect_clocked: Optional[bool]) -> None:
        ar = _session.arena()
        if self._field is not None:
            # Single field → drive its own HCP; SignalRef enforces |= vs *=.
            tgt = SignalRef(self._field_hcp(is_read=False))
            if   expect_clocked is True:  tgt |= src
            elif expect_clocked is False: tgt *= src
            else:                         ar.gen_basic_assign(tgt._ident, src._ident, src._slice, None)
        else:
            # Whole element → Rust splits the packed source across the fields.
            ar.karray_assign_element(self._ident, list(self._indices), src._ident, src._slice, expect_clocked)

    def __ior__(self, src: Union[SignalRef, "KarrayRef"]):
        self._assign(to_ref(src), expect_clocked=True)
        return _ASSIGNED

    def __imul__(self, src: Union[SignalRef, "KarrayRef"]):
        self._assign(to_ref(src), expect_clocked=False)
        return _ASSIGNED

    def _explicit_assign(self, value: Union[SignalRef, "KarrayRef"]) -> None:
        self._assign(to_ref(value), expect_clocked=None)

    # ---- augmented-assignment tails ----------------------------------------
    # `d[2][1] |= x` desugars to `d[2].__setitem__(1, d[2][1].__ior__(x))`;
    # `d[2][1].valid |= x` desugars to `setattr(d[2][1], 'valid', ....__ior__(x))`.
    # The inner op already did the work and returns the _ASSIGNED sentinel, so the
    # tail is a no-op. A real value means an explicit `=` assignment.
    def __setitem__(self, key: int, value) -> None:
        if value is _ASSIGNED:
            return
        self.__getitem__(key)._explicit_assign(value)

    def __setattr__(self, name: str, value) -> None:
        if name in KarrayRef.__slots__:
            object.__setattr__(self, name, value)
            return
        if value is _ASSIGNED:
            return
        self.__getattr__(name)._explicit_assign(value)


# ---- karray -----------------------------------------------------------------
class Karray:
    """Typed multi-dimensional array (Karray CCP) — a thin handle over the Rust
    CcpIdent (no cached layout). `shape` is the dimension tuple (e.g. `(5, 3)`);
    `fields` is the element record as `(name, width)` pairs, in declaration order,
    each materialised as its OWN hardware component. `backing` is a
    `kathryn.HwComponentType` member — `REG` / `WIRE` / `MEM_BLOCK`."""

    __slots__ = ("_ident",)

    def __init__(self, shape: Iterable[int], fields: Iterable[Tuple[str, int]],
                 backing: int = HwComponentType.REG, name: Optional[str] = None) -> None:
        name        = name or _session.auto_name("karray")
        flds        = [(str(n), int(w)) for (n, w) in fields]
        self._ident = _session.arena().mk_karray(name, [int(d) for d in shape], flds, int(backing))

    @property
    def ident(self):                          # the underlying CcpIdent
        return self._ident

    def __getitem__(self, key: int) -> KarrayRef:
        return KarrayRef(self._ident).__getitem__(key)

    def __setitem__(self, key: int, value) -> None:
        # Tail of `d[i] |= x` on a 1-D Karray, where the subscript lands directly on
        # the Karray (not a KarrayRef): the KarrayRef already did the assign and
        # returned the sentinel. A real value is an explicit `d[i] = rhs`.
        if value is _ASSIGNED:
            return
        self[key]._explicit_assign(value)
