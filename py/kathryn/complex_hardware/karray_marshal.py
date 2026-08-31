# Karray key / source marshalling helpers — everything that turns a user-facing
# value (a `[]` key, an assignment source, a select fn) into what the connector
# ships to Rust. Pure functions, no state; `KarrayRef._selectors` /
# `_assign_from` (karray_ref.py) are the callers. Aliases live in karray_types.py.

from __future__ import annotations

from typing import TYPE_CHECKING, Iterator, List, Mapping, Tuple, Union

from .._kathryn import HcpIdent
from ..signal import SignalRef, to_ref
from .karray_types import FieldSource, RawSelectFn, ReduceView, UserSelectFn

if TYPE_CHECKING:                      # annotation-only — no runtime cycle
    from .karray_ref import KarrayRef


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
