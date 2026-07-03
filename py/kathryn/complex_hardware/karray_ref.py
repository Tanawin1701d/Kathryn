# KarrayRef — a partially/fully-indexed view into a Karray (see karray.py). Holds
# only the Karray's CcpIdent plus the accumulated index/slice keys and an optional
# field name; all shape/field validation happens in Rust at resolution time.

from __future__ import annotations

from typing import Optional, Sequence, Union

from .. import _session
from ..signal import SignalRef, _ASSIGNED, to_ref


# ---- dynamic one-hot index marker -------------------------------------------
class OneHot:
    """Marks a signal as a *one-hot* dynamic Karray index: `d[oh(sel)]`. A bare
    signal index (`d[sel]`) is treated as a binary-encoded address instead."""

    __slots__ = ("sig",)

    def __init__(self, sig: SignalRef) -> None:
        if not isinstance(sig, SignalRef):
            raise TypeError("oh() expects a signal (one-hot select line)")
        self.sig = sig


def oh(sig: SignalRef) -> OneHot:
    """Wrap a select line so a Karray indexes it as a one-hot (not binary) address."""
    return OneHot(sig)


# ---- karray element / field reference ---------------------------------------
class KarrayRef:
    """A partially- or fully-indexed view into a Karray, optionally narrowed to a
    field. Holds only the Karray's CcpIdent plus the accumulated keys (int or
    slice, one per indexed dimension) and field name; shape/field validation
    happens in Rust at resolution time."""

    __slots__ = ("_ident", "_keys", "_field")

    def __init__(self, ident, keys: Optional[Sequence] = None, field: Optional[str] = None) -> None:
        object.__setattr__(self, "_ident", ident)
        object.__setattr__(self, "_keys" , list(keys) if keys else [])
        object.__setattr__(self, "_field", field)
        # Bypass our own __setattr__ (reserved for the assignment tail).
        # _keys holds one entry per indexed dim; each entry is an int or a slice:
        #   d[2]        -> _keys=[2]                  int             single element of the dim
        #   d[0:2]      -> _keys=[slice(0, 2)]        slice start:stop  range (stop exclusive)
        #   d[0:]       -> _keys=[slice(0, None)]     slice start:      open end -> to last
        #   d[:2]       -> _keys=[slice(None, 2)]     slice :stop       open start -> from 0
        #   d[:]        -> _keys=[slice(None, None)]  slice :           whole dim
        #   d[2][1]     -> _keys=[2, 1]               nested -> one key appended per [] hop
        # _field is the trailing dtype field name, or None:
        #   d[2][1].valid -> _ident=<d>, _keys=[2, 1], _field="valid".


    # ---- indexing / field selection ----------------------------------------
    def __getitem__(self, key) -> "KarrayRef":
        if self._field is not None:
            raise TypeError("cannot index into a Karray field")
        # int / slice = static; a signal = dynamic binary address; oh(signal) = dynamic one-hot.
        if not isinstance(key, (int, slice, SignalRef, OneHot)):
            raise TypeError("Karray index must be an int, slice, a signal (dynamic binary), "
                            "or oh(signal) (dynamic one-hot)")
        return KarrayRef(self._ident, self._keys + [key], None)

    def __getattr__(self, name: str) -> "KarrayRef":
        # Only reached when normal (slot) lookup fails, so this is a field name.
        # The name is validated in Rust when the ref is resolved (see karray_static_index_get_hcp).
        if name.startswith("_"):
            raise AttributeError(name)
        return KarrayRef(self._ident, self._keys, name)

    # ---- key resolution ----------------------------------------------------
    # Scalar paths (a single field / packed element) require every key to be a
    # plain int; a range slice only makes sense for a karray-to-karray region.
    def _int_indices(self) -> list:
        idx = []
        for k in self._keys:
            if isinstance(k, slice):
                raise TypeError("a sliced Karray region cannot be used as a scalar field/element")
            idx.append(int(k))
        return idx

    # Per-dimension selectors for the karray-to-karray path: int -> (i, i+1, False)
    # (collapses the dim); slice -> (start, stop|None, True) (keeps it).
    def _region_selectors(self) -> list:
        sels = []
        for key in self._keys:
            if isinstance(key, slice):
                if key.step not in (None, 1):
                    raise ValueError("Karray slice step must be 1")
                start = 0 if key.start is None else int(key.start)
                stop  = None if key.stop is None else int(key.stop)
                sels.append((start, stop, True))
            else:
                ik = int(key)
                sels.append((ik, ik + 1, False))
        return sels

    # True when any indexed dim uses a runtime signal (binary or one-hot).
    def _has_dynamic(self) -> bool:
        return any(isinstance(k, (SignalRef, OneHot)) for k in self._keys)

    # Per-dimension selectors for the dynamic-read path, in the connector's encoding:
    #   int       -> ("static", i,    None)
    #   signal    -> ("bin",    None, sig)   binary-encoded address
    #   oh(signal)-> ("onehot", None, sig)   one-hot select line
    # A range/slice cannot mix with dynamic indexing, and the index signal must be
    # whole (a sliced view carries no width here — assign it to a wire first).
    def _dyn_selectors(self) -> list:
        sels = []
        for k in self._keys:
            if isinstance(k, OneHot):
                if k.sig._is_user_assigned:
                    raise TypeError("a sliced signal cannot be a dynamic index; assign it to a wire first")
                sels.append(("onehot", None, k.sig._ident))
            elif isinstance(k, SignalRef):
                if k._is_user_assigned:
                    raise TypeError("a sliced signal cannot be a dynamic index; assign it to a wire first")
                sels.append(("bin", None, k._ident))
            elif isinstance(k, int):
                sels.append(("static", int(k), None))
            else:  # slice
                raise TypeError("a range/slice index cannot be combined with dynamic indexing")
        return sels

    # single hcp access/ref

    def _field_hcp(self, is_read: bool):
        return _session.arena().karray_static_index_get_hcp(self._ident, self._int_indices(), self._field, bool(is_read))

    # Dynamic read: resolve the selection into a scalar result Karray holding just
    # this field, then read field 0 of that result (a fresh mux-output wire).
    def _dyn_field_hcp(self):
        result_ccp, _resolved = _session.arena().karray_dynamic_index_get(
            self._ident, self._dyn_selectors(), [self._field],
        )
        return _session.arena().karray_static_index_get_hcp(result_ccp, [0], self._field, True)

    # Read hook consumed by signal.to_ref (use a field as an assignment source).
    def _to_read_ref(self) -> SignalRef:
        if self._field is None:
            raise TypeError("read a specific field (d[i][j].field), not a whole Karray element")
        if self._has_dynamic():
            return SignalRef(self._dyn_field_hcp())
        return SignalRef(self._field_hcp(is_read=True))

    # ---- assignment --------------------------------------------------------
    # Route every operator (`|=`, `*=`, `=`) through here. Cases by (target, source):
    #   target            source                  -> path
    #   element/region    Karray element/region   -> karray-to-karray copy (k2k)   d[0:2] |= s[4:6] ; d[2][1] |= s[5][3]
    #   field             Karray element/region   -> ERROR (field target, karray source not allowed)
    #   field             scalar signal/value     -> single-field assign            d[2][1].data |= sig ; d[2][1].x |= s[5].y
    #   element           {field_name: source}    -> whole-element field map        d[2][1] |= {"valid": v, "data": x}
    #   element           non-dict / non-karray   -> ERROR (raised in _assign_element_from_map)
    def _assign_from(self, src, expect_clocked: Optional[bool]) -> None:
        from .karray import Karray            # local import breaks the Karray<->KarrayRef cycle
        if isinstance(src, Karray):
            src = KarrayRef(src._ident)            # whole-array source (no keys -> full-range every dim)

        # Classify both sides up front so each branch states its own full condition.
        src_is_karray_region = isinstance(src , KarrayRef) and src._field  is None  # source: element/region, no field
        des_is_karray_region = isinstance(self, KarrayRef) and self._field is None  # target: element/region (no `.field`)

        # Dynamic-index write: a runtime signal (binary or one-hot) selects which element
        # receives `src`; non-selected elements hold (reg-backed `|=` only — the Rust layer
        # rejects `*=`). A Karray region source has no single runtime element, so it is
        # unsupported here.
        if self._has_dynamic():
            if src_is_karray_region:
                raise TypeError("dynamic-index Karray assignment cannot take a Karray region/element source")
            # `|=`/`*=` carry the clocked intent directly; a bare `=` carries none, so
            # resolve it from the destination backing (reg/memblock -> clocked, wire ->
            # combinational — which the Rust layer then rejects for a dynamic write).
            clocked = expect_clocked if expect_clocked is not None else _session.arena().karray_is_clocked(self._ident)
            if des_is_karray_region:                   # element <= {field_name: source} map
                self._assign_element_from_map_dynamic(src, clocked)
            else:                                       # field <= one scalar source
                self._assign_one_field_dynamic(to_ref(src), clocked)
            return

        # First layer branches on the DESTINATION only; the source is resolved inside each arm.
        if des_is_karray_region:                       # target is an element/region
            if src_is_karray_region:                   # region <= region : karray-to-karray copy
                _session.arena().karray_static_index_assign_k2k(
                    self._ident, self._region_selectors(),
                    src ._ident, src ._region_selectors(),  # empty list -> whole array (full-range every dim)
                    expect_clocked,
                )
            else:                                      # element <= {field_name: source} map
                self._assign_element_from_map(src, expect_clocked)
        else:                                          # target is a single field
            if src_is_karray_region:                   # field <= karray region is not permitted
                raise TypeError("karray-to-karray assignment target must be an array/element, not a field")
            self._assign_one_field(to_ref(src), expect_clocked)  # field <= one scalar source

    def _assign_one_field(self, src: SignalRef, expect_clocked: Optional[bool]) -> None:
        # Single field → drive its own HCP; SignalRef enforces |= vs *=.
        ar  = _session.arena()
        tgt = SignalRef(self._field_hcp(is_read=False))
        if   expect_clocked is True:  tgt |= src
        elif expect_clocked is False: tgt *= src
        else:                         ar.gen_basic_assign(tgt._ident, src._ident, src._slice, None)

    def _assign_element_from_map(self, src, expect_clocked: Optional[bool]) -> None:
        # Whole-element assign: connect each `{field_name: source}` entry to the field
        # of that name. Sources are matched by name in Rust (full-width connect).
        if not isinstance(src, dict):
            raise TypeError("whole-element Karray assign needs a {field_name: source} mapping "
                            "(per-field, not a packed bit-vector)")
        sources = [(str(name), to_ref(val)._ident) for name, val in src.items()]
        _session.arena().karray_static_index_assign_element(self._ident, self._int_indices(), sources, expect_clocked)

    # ---- dynamic-index assignment (runtime-selected element) ----------------
    def _assign_one_field_dynamic(self, src: SignalRef, clocked: bool) -> None:
        # Single field target: write `src` into this field of the runtime-selected element.
        sources = [(str(self._field), src._ident)]
        _session.arena().karray_dynamic_index_assign_element(
            self._ident, self._dyn_selectors(), sources, clocked)

    def _assign_element_from_map_dynamic(self, src, clocked: bool) -> None:
        # Whole-element target: connect each `{field_name: source}` entry to the field of
        # that name on the runtime-selected element (matched by name in Rust).
        if not isinstance(src, dict):
            raise TypeError("whole-element dynamic Karray assign needs a {field_name: source} mapping "
                            "(per-field, not a packed bit-vector)")
        sources = [(str(name), to_ref(val)._ident) for name, val in src.items()]
        _session.arena().karray_dynamic_index_assign_element(
            self._ident, self._dyn_selectors(), sources, clocked)

    def __ior__(self, src: Union[SignalRef, "KarrayRef", "Karray"]):
        self._assign_from(src, expect_clocked=True)
        return _ASSIGNED

    def __imul__(self, src: Union[SignalRef, "KarrayRef", "Karray"]):
        self._assign_from(src, expect_clocked=False)
        return _ASSIGNED

    def _explicit_assign(self, value: Union[SignalRef, "KarrayRef", "Karray"]) -> None:
        self._assign_from(value, expect_clocked=None)

    # ---- augmented-assignment tails ----------------------------------------
    # `d[2][1] |= x` desugars to `d[2].__setitem__(1, d[2][1].__ior__(x))`;
    # `d[2][1].valid |= x` desugars to `setattr(d[2][1], 'valid', ....__ior__(x))`.
    # The inner op already did the work and returns the _ASSIGNED sentinel, so the
    # tail is a no-op. A real value means an explicit `=` assignment.
    def __setitem__(self, key, value) -> None:
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