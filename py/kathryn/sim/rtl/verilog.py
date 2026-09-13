# Sim RTL VERILOG — what a Verilog emit looks like to the simulator.
# - The cross-module port ids (IO_WIRE_IO_IN/OUT_<name>_<id>) and their order
#   are taken out before hashing.  The backend routes those ports in a fixed
#   order now, so two emits of one design are byte-identical; the
#   normalisation stays as a guard, since a regression there would cost a
#   silent full rebuild per run.

from __future__ import annotations

import pathlib
import re
from typing import List

from .base import Rtl

_IO_PORT_ID = re.compile(r"(IO_WIRE_IO_(?:IN|OUT)_\w+?)_\d+\b")


class VerilogRtl(Rtl):
    backend_tag = "verilog"

    def sources(self) -> List[pathlib.Path]:
        return sorted(self.dir.glob("*.v"))

    @staticmethod
    def normalize(text: str) -> bytes:
        kept, ports = [], []
        for line in text.splitlines():
            line = line.strip().rstrip(",")             # the last port of a list has no comma
            if line:
                (ports if "IO_WIRE_" in line else kept).append(_IO_PORT_ID.sub(r"\1", line))
        return "\n".join(kept + sorted(ports)).encode()
