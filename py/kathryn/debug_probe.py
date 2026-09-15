# DebugProbe — a plain attribute holder the sim manifest walks like a Module.
# - NO hardware, NO ident: it only groups built signals (SignalRefs, nested
#   probes, lists, dicts, karrays, counters) under one Module attribute, so the
#   simulator reads them as `k.<attr>.<name>.value`.
# - Meant for a @dbg body, where a Module reads built idents back:
#   `self.pipe = DebugProbe(wait=self.con.pip_wait_reg, ack=self.con.master_ack)`.
#   An @init body may hold one too.
# - LIMIT: a signal on a probe resolves in the scope of the Module that holds
#   the probe, the same rule as a plain Module attribute; a sub-module's signal
#   is probed from that sub-module.

from __future__ import annotations

from typing import Any


class DebugProbe:
    """A named group of signals for the sim manifest; the attributes are the keys."""
    # `_`-prefixed attributes are skipped by the walker, like on a Module.
    def __init__(self, **attrs: Any) -> None:
        for name, value in attrs.items():
            setattr(self, name, value)
