# Shared Karray typing surface: the alias every karray file annotates against
# (CLAUDE.md §7.5 — shared aliases beat repeated unions) plus `ReduceView`, the
# object a reduce select fn receives. NOT here: key marshalling (karray_marshal.py)
# and the KarrayRef class itself (karray_ref.py).

from __future__ import annotations

from typing import TYPE_CHECKING, Any, Callable, Dict, List, Mapping, Tuple, Union

from .._kathryn import HcpIdent
from ..signal import SignalRef

if TYPE_CHECKING:                      # annotation-only — no runtime cycle
    from .karray_ref import KarrayRef

# ---- index / source / selector -----------------------------------------------

# One `[]` key: int (static) / SignalRef (dynamic) / callable (custom fn).
KarrayKey   = Union[int, SignalRef, Callable[..., Any]]

# One assignment source: scalar signal/int, a karray view, or a field map
# (values are themselves FieldSources — dicts nest for bundles).
FieldSource = Union[SignalRef, "KarrayRef", int, Mapping[str, Any]]

# ONE dimension's index in wire form: the `(kind, ints, sigs)` triple the
# connector ships to Rust, where kidx_py.rs (the single decode point) validates
# the per-kind arity and rebuilds the core's `KIdx`:
#
#   ┌──────────────────────┬──────────────────────────┬─────────────────────────────────────┐
#   │ you wrote            │ encoded triple           │ meaning in Rust (KIdx)              │
#   ├──────────────────────┼──────────────────────────┼─────────────────────────────────────┤
#   │ d[3]                 │ ("static", [3], [])      │ Static(3) — compile-time index      │
#   │ d[addr]              │ ("dyn", [], [addr_hcp])  │ Dyn(sig) — runtime mux/decode addr  │
#   │ d[fn]  (write dest)  │ ("cus", [], [en0, ...])  │ CusWe(bits) — fn pre-evaluated per  │
#   │                      │                          │ index, one 1-bit enable per element │
#   │ d[fn]  (read source) │ ("reduce", [], [])       │ CusRd — payload empty; the wrapped  │
#   │                      │                          │ select fn rides the parallel fns    │
#   │                      │                          │ list (a callable can't ride the     │
#   │                      │                          │ triple)                             │
#   └──────────────────────┴──────────────────────────┴─────────────────────────────────────┘
EncodedKIdx = Tuple[str, List[int], List[HcpIdent]]


# ---- reduce view ---------------------------------------------------------------
# Example — max-element read over a 4-element array with a "data" field:
#
#   def pick_max(a: ReduceView, b: ReduceView, level: int) -> SignalRef:
#       return a.fields["data"] >= b.fields["data"]     # 1-bit: pick `a` when high
#
#   best = d[pick_max].data
#
# The fold calls the fn once per 2:1 tree node; at the root it sees
#
#   a = ReduceView(indices=[0, 1], fields={"data": <winner of e0 vs e1>})
#   b = ReduceView(indices=[2, 3], fields={"data": <winner of e2 vs e3>})
#   level = 1                                  # 0 = the leaf pairs
#
# and returns a 1-bit "pick a" signal. Returning a tuple instead, e.g.
#   (sel, {"sum": a.fields["sum"] + b.fields["sum"]})
# also replaces/appends the named field carried on the merged node (a running
# sum here), which the NEXT level then sees in its `.fields`.
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


# ---- reduce-select fn signatures ----------------------------------------------
# Named pieces first, so the two Callable shapes below read argument by argument.

FlatFields  = List[Tuple[str, HcpIdent]]   # a subtree's fields as flat (name, hcp) pairs
CoveredIdxs = List[int]                    # the folding dim's indices a subtree covers

# Rust reduce-fold callback ABI (see arena_impl_ccp_karray_py.rs::reduce_select):
# (a_fields, a_indices, b_fields, b_indices, level) -> (select_hcp, extras).
# Example — one leaf-pair call (elements 0 vs 1, "data" field, level 0):
#
#   raw(a_fields=[("data", h_e0)], a_indices=[0],
#       b_fields=[("data", h_e1)], b_indices=[1],
#       level=0)
#   -> (h_ge_expr, [])            # hcp of the `>=` expression; no extras
#
RawSelectFn = Callable[
    [FlatFields, CoveredIdxs,      # subtree a
     FlatFields, CoveredIdxs,      # subtree b
     int],                         # level in the 2:1 fold tree (0 = leaves)
    Tuple[HcpIdent, FlatFields],   # pick-a select + replace/append extras
]

# The user's select fn: (a, b, level) -> select, or (select, {name: extra}).
# It never sees the raw ABI — karray_marshal._wrap_select adapts: same call as
# above arrives as
#
#   pick_max(a=ReduceView(indices=[0], fields={"data": SignalRef(h_e0)}),
#            b=ReduceView(indices=[1], fields={"data": SignalRef(h_e1)}),
#            level=0)
#   -> a.fields["data"] >= b.fields["data"]           # or (select, {"sum": ...})
#
UserSelectFn = Callable[
    [ReduceView, ReduceView, int],
    Union[SignalRef, Tuple[SignalRef, Dict[str, SignalRef]]],
]
