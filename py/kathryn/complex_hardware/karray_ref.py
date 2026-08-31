# KarrayRef — a partially/fully-indexed view into a Karray (see karray.py).
# Holds ONLY the Karray's CcpIdent + accumulated per-dim keys + an optional
# flattened field name; all layout validation happens in Rust at resolve time.
# Split companions: shared aliases + ReduceView in karray_types.py; key/source
# marshalling helpers in karray_marshal.py.
#
# ONE unified index — each `[]` hop selects ONE dimension; every kind collapses
# it to a single element (NO ranges — every dim must be indexed):
#   d[3]    int       static index
#   d[sig]  signal    dynamic binary-encoded address
#   d[fn]   callable  custom fn — direction-split at the statement:
#     - WRITE dest : `fn(i) -> 1-bit enable`, called once per index; the write
#       lands only where high (one-hot: `d[lambda i: sel[i]] |= ...`).
#     - READ source: the dim folds through a REDUCE tree — `fn(a, b, level) ->
#       pick_a` per 2:1 node over two ReduceViews; may return
#       `(select, {name: signal})` extras that replace same-named carried
#       fields (max element: `d[lambda a, b, l: a.fields["data"] >=
#       b.fields["data"]].data`).
#
# Fields may be BUNDLES (karray_field.py): attribute hops chain the flat leaf
# name (`d[0].pos.x` -> "pos_x"); dict sources nest (`{"pos": {"x": a}}`).

from __future__ import annotations

from typing import List, Optional, Sequence, Tuple, Union

from .. import _session
from .._kathryn import CcpIdent
from ..signal import SignalRef, _ASSIGNED, _Assigned
from .karray_marshal import (
    _check_key_type,
    _enable_bit,
    _flatten_field_map,
    _to_operand,
    _whole_signal,
    _wrap_select,
)
from .karray_types import EncodedKIdx, FieldSource, KarrayKey, RawSelectFn


# ---- karray element / field reference ---------------------------------------
class KarrayRef:
    """A partially- or fully-indexed view into a Karray, optionally narrowed to
    a (possibly bundle-nested) field. Holds only the Karray's CcpIdent plus the
    accumulated keys and flattened field name; layout validation happens in Rust."""

    __slots__ = ("_ident", "_keys", "_field")
    _ident : CcpIdent
    _keys  : List[KarrayKey]
    _field : Optional[str]

    def __init__(self, ident: CcpIdent, keys: Optional[Sequence[KarrayKey]] = None, field: Optional[str] = None) -> None:
        # Bypass our own __setattr__ (reserved for the assignment tail).
        object.__setattr__(self, "_ident", ident)
        object.__setattr__(self, "_keys" , list(keys) if keys else [])
        object.__setattr__(self, "_field", field)

    # ---- indexing / field selection ----------------------------------------
    def __getitem__(self, key: KarrayKey) -> "KarrayRef":
        if self._field is not None:
            raise TypeError("cannot index into a Karray field")
        _check_key_type(key)
        return KarrayRef(self._ident, self._keys + [key], None)

    def __getattr__(self, name: str) -> "KarrayRef":
        # Only reached when normal (slot) lookup fails, so this is a field name.
        # Chained attributes walk INTO a bundle by joining with '_' — the flat
        # leaf name the Rust layout stores. Validated in Rust at resolve time.
        if name.startswith("_"):
            raise AttributeError(name)
        merged = name if self._field is None else f"{self._field}_{name}"
        return KarrayRef(self._ident, self._keys, merged)

    # ---- key resolution -----------------------------------------------------
    def _selectors(self, for_read: bool) -> Tuple[List[EncodedKIdx], List[Optional[RawSelectFn]]]:
        # Encode the raw keys into the connector's (kind, ints, sigs) triples,
        # plus the parallel per-dim fns list. A callable key is direction-split HERE:
        #
        #   raw key         | kind     | ints | sigs                 | fns[dim]
        #   ----------------|----------|------|----------------------|-----------------
        #   int             | "static" | [i]  | []                   | None
        #   SignalRef       | "dyn"    | []   | [addr]               | None
        #   callable, read  | "reduce" | []   | []                   | wrapped select fn
        #   callable, write | "cus"    | []   | fn(i) per-idx enable | None
        shape: Optional[List[int]] = None                        # dim sizes, fetched lazily (first "cus" key only)
        sels : List[EncodedKIdx]           = []                  # one (kind, ints, sigs) triple per dim
        fns  : List[Optional[RawSelectFn]] = []                  # parallel to sels: select fn or None per dim
        for dim, key in enumerate(self._keys):
            fn_entry = None
            if isinstance(key, bool):                   # bool is an int subclass — never an index
                raise TypeError("Karray index must not be a bool")
            if isinstance(key, int):
                sels.append(("static", [int(key)], []))          # compile-time index: ship just the number
            elif isinstance(key, SignalRef):
                sels.append(("dyn", [], [_whole_signal(key)._ident]))  # runtime address (slice materialised first)
            elif callable(key):
                if for_read:
                    sels.append(("reduce", [], []))              # marker only — the fn itself rides in fns
                    fn_entry = _wrap_select(key)                 # adapt user (a, b, level) fn to the raw ABI
                else:
                    if shape is None:                            # ask Rust for the dim sizes, once
                        shape = _session.arena().karray_shape(self._ident)
                    if dim >= len(shape):
                        raise ValueError(f"too many indices: got {len(self._keys)} for a {len(shape)}-D Karray")
                    # user fn evaluated NOW, once per index of this dim -> one 1-bit write enable each
                    bits = [_enable_bit(key(i), dim, i)._ident for i in range(shape[dim])]
                    sels.append(("cus", [], bits))
            else:                                       # unreachable: __getitem__ validated the key
                raise TypeError(f"unsupported Karray index: {key!r}")
            fns.append(fn_entry)                                 # keeps fns index-aligned with sels
        return sels, fns

    # Read hook consumed by signal.to_ref (use a field as an assignment source).
    # Every dimension must be indexed (static / dynamic / reduce).
    def _to_read_ref(self) -> SignalRef:
        if self._field is None:
            raise TypeError("read a specific field (d[i][j].field), not a whole Karray element")
        sels, fns = self._selectors(for_read=True)
        hcp = _session.arena().karray_read_field_hcp(self._ident, sels, fns, self._field)
        return SignalRef(hcp)

    # ---- assignment --------------------------------------------------------
    # Route every operator through here. Cases by (target, source):
    #   target          source                 -> path
    #   element/region  Karray element/region  -> karray-to-karray copy (k2k)
    #   field           Karray element/region  -> ERROR
    #   field           scalar signal/int      -> single-field assign
    #   field (bundle)  {sub_field: source}    -> bundle map assign (prefixed)
    #   element         {field_name: source}   -> per-field map assign (dicts nest)
    #   element         scalar signal/int      -> sole-field assign (single-field Karray only)
    def _assign_from(self, src: FieldSource, expect_clocked: Optional[bool]) -> None:
        from .karray import Karray            # local import breaks the Karray<->KarrayRef cycle
        if expect_clocked is None:
            raise TypeError("Karray assignment requires `|=` (clocked) or `*=` (combinational), "
                            "not a bare `=`")
        if isinstance(src, Karray):
            src = KarrayRef(src._ident)            # no keys -> the Rust rank check rejects it clearly

        if isinstance(src, KarrayRef) and src._field is None:
            # ---- k2k: element/region source -> paired region copy ----
            if self._field is not None:
                raise TypeError("karray-to-karray assignment target must be an array/element, not a field")
            dst_sels, _        = self._selectors(for_read=False)
            src_sels, src_fns  = src ._selectors(for_read=True)
            _session.arena().karray_assign_k2k(
                self._ident, dst_sels,
                src ._ident, src_sels, src_fns,
                expect_clocked,
            )
            return

        # ---- scalar path: field ref / signal / expr / int / {field: source} map ----
        if isinstance(src, dict):
            # element target: names are field names; bundle-field target: names
            # are sub-fields, prefixed with the bundle path. Dicts nest either way.
            prefix  = "" if self._field is None else self._field + "_"
            sources = list(_flatten_field_map(src, prefix))
        else:
            sources = [(self._field, _to_operand(src))]   # field name, or None -> the sole field
        dst_sels, _ = self._selectors(for_read=False)
        _session.arena().karray_assign_hcps(self._ident, dst_sels, sources, expect_clocked)

    def __ior__(self, src: FieldSource) -> _Assigned:
        self._assign_from(src, expect_clocked=True)
        return _ASSIGNED

    def __imul__(self, src: FieldSource) -> _Assigned:
        self._assign_from(src, expect_clocked=False)
        return _ASSIGNED

    # ---- augmented-assignment tails ----------------------------------------
    # `d[2][1] |= x` desugars to `d[2].__setitem__(1, d[2][1].__ior__(x))`;
    # `d[2][1].valid |= x` desugars to `setattr(d[2][1], 'valid', ....__ior__(x))`.
    # The inner op already did the work and returns the _ASSIGNED sentinel, so
    # the tail is a no-op. A real value means a bare `=`, which is rejected.
    def __setitem__(self, key: KarrayKey, value: Union[_Assigned, FieldSource]) -> None:
        if value is _ASSIGNED:
            return
        self.__getitem__(key)._assign_from(value, expect_clocked=None)

    def __setattr__(self, name: str, value: Union[_Assigned, FieldSource]) -> None:
        if name in KarrayRef.__slots__:
            object.__setattr__(self, name, value)
            return
        if value is _ASSIGNED:
            return
        self.__getattr__(name)._assign_from(value, expect_clocked=None)
