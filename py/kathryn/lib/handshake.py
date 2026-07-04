# Decoupled — the standard valid/ready handshake bundle. The producer drives
# `valid` + payload and must hold them until `ready`; the consumer drives
# `ready`. A transfer ("fire") happens on a cycle where both are high.
#
# Directionality: within one module both sides just share the object (producer
# assigns valid/payload, consumer assigns ready). At a module's IO boundary use
# mark_producer_io / mark_consumer_io — the two differ only in which way
# `ready` points.

from __future__ import annotations

from typing import Dict

from ..signal import SignalRef
from .bundle import Bundle

_CTRL_FIELDS = ("valid", "ready")


class Decoupled(Bundle):
    def __init__(self, name: str, payload: Dict[str, int]) -> None:
        for f in _CTRL_FIELDS:
            if f in payload:
                raise ValueError(f"Decoupled: payload may not shadow control field {f!r}")
        super().__init__(name, {"valid": 1, "ready": 1, **payload})

    def payload_names(self) -> list[str]:
        return [f for f in self.field_names() if f not in _CTRL_FIELDS]

    def fire(self) -> SignalRef:
        # 1-bit expression: this cycle a transfer takes place.
        return self.valid & self.ready

    # ---- IO marking (direction depends on which side this module plays) -----
    def mark_producer_io(self) -> "Decoupled":
        # This module PRODUCES: valid + payload are outputs, ready is an input.
        self.valid.mark_output(f"{self.name}_valid")
        self.ready.mark_input (f"{self.name}_ready")
        for f in self.payload_names():
            getattr(self, f).mark_output(f"{self.name}_{f}")
        return self

    def mark_consumer_io(self) -> "Decoupled":
        # This module CONSUMES: valid + payload are inputs, ready is an output.
        self.valid.mark_input  (f"{self.name}_valid")
        self.ready.mark_output (f"{self.name}_ready")
        for f in self.payload_names():
            getattr(self, f).mark_input(f"{self.name}_{f}")
        return self
