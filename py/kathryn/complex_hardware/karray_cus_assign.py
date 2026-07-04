# Karray custom dynamic-write surface: the spread marker (`Spread`), the per-element
# view handed to the user write fn (`WriteView`), and the `_KarrayCusAssignMixin` that
# adds `Karray.cus_dynamic_assign`. Split out of karray.py; `Karray` mixes this in.

from __future__ import annotations

from .. import _session
from ..signal import to_ref


# ---- cus_dynamic_assign: spread marker + the view handed to the write fn ------
class _Spread:
    """A spread dimension for `cus_dynamic_assign`: fan out over the whole extent and
    let the write function decide each element's write-enable. An int pins the dim
    instead (writes only that index's slice of the array); a `slice(a, b)` fans out
    over just the half-open sub-range `[a, b)`."""
    __slots__ = ()

    def __repr__(self) -> str:
        return "Spread"


Spread = _Spread()


class WriteView:
    """The element handed to a `cus_dynamic_assign` write function. `.coord` is the
    element's static coordinate (a list of ints). Build a 1-bit write-enable from it
    (and any runtime index signal you close over) and return that signal; the element
    is written from the supplied `src` only when the enable is high."""
    __slots__ = ("coord",)

    def __init__(self, coord) -> None:
        self.coord = coord        # element's static coordinate (list of ints)


class _KarrayCusAssignMixin:
    """Custom callback-driven dynamic-write methods mixed into `Karray`. Relies on the
    host providing `self._ident`."""

    __slots__ = ()

    # ---- custom callback-driven dynamic assign (algorithm lives in Rust) ----
    def cus_dynamic_assign(self, dims, src, write_fn, clocked=True):
        """Write `src` into elements chosen by a user `write_fn`, exposing the element's
        static coordinate. `dims` has one entry per dimension: an int pins it; a
        `slice(a, b)` fans out over the half-open sub-range `[a, b)` (open ends allowed,
        e.g. `slice(2, None)`); `Spread` fans out over the whole dim. `src` is a
        `{field_name: source}` mapping (sources matched to fields by name, full-width).
        For each fanned-out element the core calls `write_fn(view)`,
        where `view.coord` is the element's static coordinate; it must return a 1-bit
        write-enable signal. The element is written from `src` only when that enable is
        high (others hold) — so close over whatever runtime index signal you like
        (binary, one-hot, range, …) to build the enable. Reg-backed `|=` only.

        The fan-out ALGORITHM runs in Rust (`karray_dynamic_cus_assign_run::write_run`); this
        method only classifies `dims` and wraps `write_fn` so the Rust core can call it
        back per element."""
        enc     = self._classify_write_dims(dims)
        sources = [(str(name), to_ref(val)._ident) for name, val in src.items()]
        raw_fn  = self._wrap_write(write_fn)
        _session.arena().karray_dynamic_cus_assign(self._ident, enc, raw_fn, sources, clocked)

    # Turn `dims` into the per-dimension `(start, stop, is_range)` encoding the Rust
    # core wants (the same triple as the static side):
    #   int         -> (i, None, False)     pins the dim to index i
    #   slice(a, b) -> (a or 0, b, True)    fans the dim out over the half-open [a, b)
    #   Spread      -> (0, None, True)      fans the whole dim out
    # Returns the encoding list (one entry per dimension); needs >= 1 fan-out dim.
    def _classify_write_dims(self, dims):
        enc, have_fanout = [], False
        for i, d in enumerate(dims):
            if isinstance(d, _Spread):
                enc.append((0, None, True))
                have_fanout = True
            elif isinstance(d, slice):
                if d.step not in (None, 1):
                    raise ValueError("cus_dynamic_assign range slice must have step 1")
                start = 0 if d.start is None else int(d.start)
                stop  = None if d.stop is None else int(d.stop)
                enc.append((start, stop, True))
                have_fanout = True
            elif isinstance(d, int):
                enc.append((int(d), None, False))
            else:
                raise TypeError("cus_dynamic_assign dim must be an int (pin), a slice (range), or Spread")
        if not have_fanout:
            raise ValueError("cus_dynamic_assign needs at least one Spread or range dimension")
        return enc

    # Wrap a user write fn into the raw callback the Rust core invokes per element: build
    # the WriteView from the raw coord, call the user fn, return the write-enable HcpIdent.
    @staticmethod
    def _wrap_write(user_fn):
        def _raw(coord):
            view = WriteView(list(coord))
            return to_ref(user_fn(view))._ident
        return _raw
