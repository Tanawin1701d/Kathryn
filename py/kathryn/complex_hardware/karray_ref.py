# KarrayRef — a partially/fully-indexed view into a Karray (see karray.py).
# Holds ONLY the Karray's CcpIdent + accumulated per-dim keys + an optional
# flattened field name; all layout validation happens in Rust at resolve time.
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

from typing import Any, Callable, Dict, Iterator, List, Mapping, Optional, Sequence, Tuple, Union

from .. import _session
from .._kathryn import CcpIdent, HcpIdent
from ..signal import SignalRef, _ASSIGNED, _Assigned, to_ref

# ---- shared types -------------------------------------------------------------

# One `[]` key: int (static) / SignalRef (dynamic) / callable (custom fn).
KarrayKey   = Union[int, SignalRef, Callable[..., Any]]

# One assignment source: scalar signal/int, a karray view, or a field map
# (values are themselves FieldSources — dicts nest for bundles).
FieldSource = Union[SignalRef, "KarrayRef", int, Mapping[str, Any]]

# The connector's per-dim selector triple `(kind, ints, sigs)`. Per-kind arity
# rule lives in kidx_py.rs: "static" -> 1 int, "dyn" -> 1 sig, "cus" -> n sigs,
# "reduce" -> none (its select fn rides the parallel fns list).
Selector    = Tuple[str, List[int], List[HcpIdent]]

# ---- reduce-select fn signatures ----------------------------------------------
# Named pieces first, so the two Callable shapes below read argument by argument.

FlatFields  = List[Tuple[str, HcpIdent]]   # a subtree's fields as flat (name, hcp) pairs
CoveredIdxs = List[int]                    # the folding dim's indices a subtree covers

# Rust reduce-fold callback ABI (see arena_impl_ccp_karray_py.rs::reduce_select):
# (a_fields, a_indices, b_fields, b_indices, level) -> (select_hcp, extras).
RawSelectFn = Callable[
    [FlatFields, CoveredIdxs,      # subtree a
     FlatFields, CoveredIdxs,      # subtree b
     int],                         # level in the 2:1 fold tree (0 = leaves)
    Tuple[HcpIdent, FlatFields],   # pick-a select + replace/append extras
]

# The user's select fn: (a, b, level) -> select, or (select, {name: extra}).
UserSelectFn = Callable[
    ["ReduceView", "ReduceView", int],
    Union[SignalRef, Tuple[SignalRef, Dict[str, SignalRef]]],
]


# ---- reduce view --------------------------------------------------------------
class ReduceView:
    """One subtree handed to a reduce select fn. `.fields` maps each carried
    field name to its current SignalRef (a leaf element's field, a mux-output
    wire, or a prior level's extra); `.indices` is the list of this dimension's
    indices the subtree covers."""
    __slots__ = ("indices", "fields")
    indices : List[int]             # covered indices of the folding dim
    fields  : Dict[str, SignalRef]  # field name -> current SignalRef

    def __init__(self, indices: List[int], fields: Dict[str, SignalRef]) -> None:
        self.indices = indices
        self.fields  = fields


# ---- key classification (the ONE Python home of the four index kinds) --------

def _check_key_type(key: object) -> None:
    # Validate a single [] key at subscript time so errors point at the use site.
    if isinstance(key, bool):
        raise TypeError("Karray index must not be a bool")
    if isinstance(key, (slice, tuple)):
        raise TypeError("Karray ranges are not supported — index exactly one element per dimension")
    if isinstance(key, (int, SignalRef)) or callable(key):
        return
    raise TypeError("Karray index must be an int (static), a signal (dynamic binary), "
                    "or a callable (custom fn)")


def _whole_signal(ref: SignalRef) -> SignalRef:
    # A sliced view (`sig[hi, lo]`) carries its slice only on the Python side;
    # materialise it into a real expression so no bits are silently dropped when
    # only the ident crosses the boundary.
    if ref._is_user_assigned:
        return ref.extend(ref._slice.stop - ref._slice.start)
    return ref


def _enable_bit(x: Union[SignalRef, "KarrayRef"], dim: int, idx: int) -> SignalRef:
    # One custom write-enable: whatever the user's fn returned, resolved to a
    # whole 1-bit signal.
    ref   = to_ref(x)
    width = ref._slice.stop - ref._slice.start
    if width != 1:
        raise TypeError(f"custom write index fn for dim {dim} must return a 1-bit enable, "
                        f"got {width} bits at index {idx}")
    return _whole_signal(ref)


def _wrap_select(user_fn: UserSelectFn) -> RawSelectFn:
    # Adapter between the Rust reduce fold and the user's select fn. The engine
    # calls `_raw` per compared pair with RAW handles; `_raw` dresses them up as
    # ReduceViews, calls the user fn, and undresses the result back to handles.
    def _raw(a_fields  : List[Tuple[str, HcpIdent]],
             a_indices : List[int],
             b_fields  : List[Tuple[str, HcpIdent]],
             b_indices : List[int],
             level     : int,
             ) -> Tuple[HcpIdent, List[Tuple[str, HcpIdent]]]:
        a = ReduceView(a_indices, {n: SignalRef(h) for (n, h) in a_fields})
        b = ReduceView(b_indices, {n: SignalRef(h) for (n, h) in b_fields})

        ret = user_fn(a, b, level)
        if isinstance(ret, tuple):
            select, extras = ret
        else:
            select, extras = ret, {}

        return (_whole_signal(to_ref(select))._ident,
                [(str(n), _whole_signal(to_ref(s))._ident) for n, s in extras.items()])
    return _raw


def _to_operand(x: Union[SignalRef, "KarrayRef", int]) -> Union[int, HcpIdent]:
    # An assignment source for the connector: a raw int passes through (wrapped
    # into a field-width val in Rust); anything else resolves to a whole signal.
    if isinstance(x, int) and not isinstance(x, bool):
        return x
    return _whole_signal(to_ref(x))._ident


def _flatten_field_map(mapping: Mapping[str, FieldSource], prefix: str = "") -> Iterator[Tuple[str, Union[int, HcpIdent]]]:
    # Bundle-aware source map: nested dicts flatten into the underscore-joined
    # leaf names the Rust layout stores ({"pos": {"x": a}} -> ("pos_x", a)).
    for name, value in mapping.items():
        key = f"{prefix}{name}"
        if isinstance(value, dict):
            yield from _flatten_field_map(value, key + "_")
        else:
            yield (key, _to_operand(value))


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
    # Encode the raw keys into the connector's (kind, ints, sigs) triples, plus
    # the parallel per-dim fns list (reduce selects on the read side). A callable
    # key is direction-split HERE: write -> fn(i) evaluated per index into "cus"
    # enables; read -> a "reduce" marker plus the wrapped select fn.
    def _selectors(self, for_read: bool) -> Tuple[List[Selector], List[Optional[RawSelectFn]]]:
        shape: Optional[List[int]] = None
        sels : List[Selector]              = []
        fns  : List[Optional[RawSelectFn]] = []
        for dim, key in enumerate(self._keys):
            fn_entry = None
            if isinstance(key, bool):                   # bool is an int subclass — never an index
                raise TypeError("Karray index must not be a bool")
            if isinstance(key, int):
                sels.append(("static", [int(key)], []))
            elif isinstance(key, SignalRef):
                sels.append(("dyn", [], [_whole_signal(key)._ident]))
            elif callable(key):
                if for_read:
                    sels.append(("reduce", [], []))
                    fn_entry = _wrap_select(key)
                else:
                    if shape is None:
                        shape = _session.arena().karray_shape(self._ident)
                    if dim >= len(shape):
                        raise ValueError(f"too many indices: got {len(self._keys)} for a {len(shape)}-D Karray")
                    bits = [_enable_bit(key(i), dim, i)._ident for i in range(shape[dim])]
                    sels.append(("cus", [], bits))
            else:                                       # unreachable: __getitem__ validated the key
                raise TypeError(f"unsupported Karray index: {key!r}")
            fns.append(fn_entry)
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
