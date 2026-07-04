# Bundle — a named group of wires treated as one interface. Kathryn has no
# native record/struct signal; a Bundle simply declares one wire per field
# (named "<bundle>_<field>") into the currently open module scope and offers
# group operations (bulk connect, bulk IO marking).
#
# Within one module, two fragments "connect" by simply sharing the Bundle
# object — one drives a field, the other reads it. `connect_from` is for
# bridging two separately created bundles of the same shape.

from __future__ import annotations

from typing import Dict

from ..hw_component import wire
from ..signal import SignalRef


class Bundle:
    def __init__(self, name: str, fields: Dict[str, int]) -> None:
        if not fields:
            raise ValueError("Bundle: needs at least one field")
        for reserved in ("_name", "_fields"):
            if reserved in fields:
                raise ValueError(f"Bundle: field name {reserved!r} is reserved")
        self._name   = str(name)
        self._fields = dict(fields)
        for f, w in self._fields.items():
            setattr(self, f, wire(int(w), f"{self._name}_{f}"))

    # ---- introspection -----------------------------------------------------
    @property
    def name(self) -> str:
        return self._name

    def field_names(self) -> list[str]:
        return list(self._fields)

    def field(self, name: str) -> SignalRef:
        return getattr(self, name)

    # ---- group operations ----------------------------------------------------
    def connect_from(self, other: "Bundle") -> None:
        # Field-by-field comb assign from a same-shaped bundle (all fields forward).
        if set(self._fields) != set(other._fields):
            raise ValueError(
                f"connect_from: field mismatch {sorted(self._fields)} vs {sorted(other._fields)}")
        for f in self._fields:
            dst = getattr(self, f)
            dst *= getattr(other, f)

    def mark_inputs(self) -> "Bundle":
        # Expose every field as a top-level input port ("<bundle>_<field>").
        for f in self._fields:
            getattr(self, f).mark_input(f"{self._name}_{f}")
        return self

    def mark_outputs(self) -> "Bundle":
        # Expose every field as a top-level output port ("<bundle>_<field>").
        for f in self._fields:
            getattr(self, f).mark_output(f"{self._name}_{f}")
        return self
