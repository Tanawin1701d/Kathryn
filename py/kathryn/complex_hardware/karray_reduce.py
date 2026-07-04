# Karray reduce surface: the fold marker (`Reduce`), the per-pair view handed to the
# user select fn (`ReduceView`), and the `_KarrayReduceMixin` that adds `Karray.reduce`.
# Split out of karray.py to keep the core handle small; `Karray` mixes this in.

from __future__ import annotations

from .. import _session
from ..signal import SignalRef, to_ref
from .karray_ref import KarrayRef


# ---- reduce: fold marker + the view handed to the user select function -------
class _Reduce:
    """A folded (reduced) Karray dimension, optionally carrying its own select fn.
    Use the bare singleton `Reduce` together with a global `select_fn`, or `Reduce(fn)`
    to attach a per-dimension select fn (so each folded dim reduces with its own rule).
    Folded dims are reduced innermost-first (highest dim index first)."""
    __slots__ = ("fn",)

    def __init__(self, fn=None) -> None:
        self.fn = fn

    def __call__(self, fn) -> "_Reduce":
        return _Reduce(fn)

    def __repr__(self) -> str:
        return "Reduce" if self.fn is None else f"Reduce({getattr(self.fn, '__name__', self.fn)})"


Reduce = _Reduce()


class ReduceView:
    """One subtree handed to a reduce select function. `.fields` maps each carried
    field name to its current SignalRef (a leaf element's field, a mux-output wire,
    or a prior layer's extra); `.indices` is the list of element coordinates this
    subtree covers."""
    __slots__ = ("indices", "fields")

    def __init__(self, indices, fields) -> None:
        self.indices = indices    # list of element coordinates (covered)
        self.fields  = fields     # dict: field name -> SignalRef


class _KarrayReduceMixin:
    """Reduce methods mixed into `Karray`. Relies on the host providing `self._ident`
    and the `__karray_fields__` class attribute."""

    __slots__ = ()

    # ---- generic callback-driven reduce (algorithm lives in Rust) ----------
    def reduce(self, dims, select_fn=None, fields=None, request_index=False):
        """Reduce elements to a single winner via user select function(s). `dims` has
        one entry per dimension: an int pins it; `Reduce` / `Reduce(fn)` folds it. Each
        folded dim reduces with its own `Reduce(fn)` (or the shared `select_fn`), and
        folded dims are reduced innermost-first (e.g. `[Reduce(fn_row), Reduce(fn_col)]`
        reduces cols within each row, then rows).

        For each compared pair the dim's fn is called as `fn(a, b, level)`, where `a`/`b`
        are `ReduceView`s (`.fields` dict, `.indices`). It returns either a 1-bit signal
        (true picks `a`), or `(select, {name: signal})` whose extras are added to the
        merged node and visible to the next layer's fn.

        `fields` limits which karray fields are carried (default: all). Returns the
        winner as a scalar `KarrayRef` (read as `winner.field`); with
        `request_index=True` returns `(winner, coords)` where `coords` is a list of the
        winner's index signal per folded dim (in dimension order). Reg/Wire only.

        The reduce ALGORITHM (nesting, the 2:1 tree, extras, per-dim coords) runs in
        Rust (`karray_dynamic_reduce_get::reduce_run`); this method only classifies `dims`
        and wraps each select fn so the Rust core can call it back."""
        enc, raw_fns = self._classify_reduce_dims(dims, select_fn)
        kfields      = list(fields) if fields is not None else [name for (name, _w) in type(self).__karray_fields__]

        res_ccp, coord_hcps = _session.arena().karray_dynamic_reduce_get(self._ident, enc, raw_fns, kfields, bool(request_index))
        res = KarrayRef(res_ccp, [0])                    # scalar winner -> read as winner.field
        if request_index:
            return res, [SignalRef(h) for h in coord_hcps]
        return res

    # Turn `dims` into the per-dimension encoding the Rust core wants:
    #   int       -> (enc=index,  fn=None)            pins the dim
    #   Reduce/(fn)-> (enc=None,   fn=raw callback)   folds the dim
    # Returns (enc, raw_fns), both one entry per dimension.
    def _classify_reduce_dims(self, dims, select_fn):
        enc, raw_fns, have_fold = [], [], False
        for i, d in enumerate(dims):
            if isinstance(d, _Reduce):
                fn = d.fn or select_fn
                if fn is None:
                    raise TypeError(f"reduce dim {i}: give a select fn via Reduce(fn) or the select_fn arg")
                enc.append(None)
                raw_fns.append(self._wrap_select(fn))
                have_fold = True
            elif isinstance(d, int):
                enc.append(int(d))
                raw_fns.append(None)
            else:
                raise TypeError("reduce dim must be an int (pin) or Reduce / Reduce(fn) (fold)")
        if not have_fold:
            raise ValueError("reduce needs at least one Reduce (folded) dimension")
        return enc, raw_fns

    # Wrap a user select fn into the raw callback the Rust core invokes: build the
    # ReduceViews, call the user fn, return (select_hcp, [(name, extra_hcp)]).
    @staticmethod
    def _wrap_select(user_fn):
        # Adapter between the Rust core and the user's select fn. The Rust core calls
        # `_raw` per compared pair with RAW handles (HcpIdent), not DSL objects; `_raw`
        # dresses them up for the user and undresses the user's result back to handles.
        def _raw(a_fields, a_indices, b_fields, b_indices, level):
            # raw side -> friendly: each side's fields arrive as [(name, HcpIdent)];
            # wrap them as SignalRefs so the user can write `a.fields['data'] >= ...`.
            a = ReduceView(a_indices, {n: SignalRef(h) for (n, h) in a_fields})
            b = ReduceView(b_indices, {n: SignalRef(h) for (n, h) in b_fields})

            # call the user's select fn; it returns either a bare select signal, or
            # `(select, {name: signal})` to also carry extra wires to the next layer.
            ret = user_fn(a, b, level)
            if isinstance(ret, tuple):
                select, extras = ret
            else:
                select, extras = ret, {}

            # friendly side -> raw: hand the Rust core back plain HcpIdents — the 1-bit
            # select-left, and the extras as a [(name, HcpIdent)] list. `to_ref` accepts
            # a SignalRef/expr (and a Karray field read) and `._ident` is its HcpIdent.
            return to_ref(select)._ident, [(str(n), to_ref(s)._ident) for n, s in extras.items()]
        return _raw
