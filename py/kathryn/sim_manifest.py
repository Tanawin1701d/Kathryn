# Sim-assist WRITER — walks the top Module's attribute tree at emit time and
# dumps an attribute-name -> emitted-name manifest next to the generated HDL.
# Counterpart: sim_assist.py (the sim-side reader; owns the shared constants).
# - Called by _session.emit_verilog BEFORE the arena moves into the backend —
#   the name/layout queries here need the live arena.
# - Names come from a per-backend SimNamer, NEVER off the raw ident: each
#   backend is the sole authority on what it emits (Verilog routes through
#   arena_ext_vb.rs::hcp_sim_name_vb / module_sim_name_vb, so e.g. an IoWire's
#   explicit port name holds). A future backend (vhdl, chisel, ...) adds one
#   SimNamer subclass + its own arena name queries; the walk itself is shared.
# - Visibility rule: only values stored as (non-underscore) Module attributes
#   are harvested. A signal held in a local variable is out of scope by design;
#   it stays reachable through the raw `dut` by its emitted name.

from __future__ import annotations

import json
import os
from itertools import product
from typing import Any, Dict, Optional

from . import _session
from .complex_hardware.counter import counter
from .complex_hardware.karray import Karray
from .hw_component import mem_blk
from .module import Module
from .signal import SignalRef
from .sim_assist import SIM_MANIFEST_FILE


# ---- per-backend naming ------------------------------------------------------

class SimNamer:
    """Resolves idents to the names ONE backend emits. Subclass per backend."""
    backend: str

    # LIMIT: idents are runtime-built PyO3 classes (HcpIdent / ModuleIdent), so
    # they are typed as Any here (no .pyi stub by project convention).
    def hcp_name   (self, hcp_i   : Any) -> str: raise NotImplementedError
    def module_name(self, module_i: Any) -> str: raise NotImplementedError


class VerilogSimNamer(SimNamer):
    backend = "verilog"

    def hcp_name   (self, hcp_i   : Any) -> str: return _session.arena().hcp_verilog_name(hcp_i)
    def module_name(self, module_i: Any) -> str: return _session.arena().module_verilog_name(module_i)


# One namer per emitting backend; emit entry points pick theirs by key.
SIM_NAMERS: Dict[str, type[SimNamer]] = {"verilog": VerilogSimNamer}


# ---- attribute -> manifest node ------------------------------------------------

def _attr_node(value: object, path: str, namer: SimNamer, visiting: set) -> Optional[Dict[str, Any]]:
    # One attribute value -> its manifest node, or None for non-hardware values:
    #
    #   attribute example               | node kind | reader (KSim) access
    #   --------------------------------|-----------|--------------------------------
    #   self.sub  = Child()             | "module"  | k.sub.<child attr>
    #   self.mem  = mem_blk(8, 4)       | "signal"  | k.mem[i].value (memory array)
    #   self.x    = reg(8)              | "signal"  | k.x.value      (read AND force)
    #   self.hi   = self.x[7, 4]        | "slice"   | k.hi.value     (read-only window)
    #   self.rf   = Rf(REG, (2,), "rf") | "karray"  | k.rf[1].data.value
    #   self.cnt  = counter(8)          | "counter" | k.cnt.value    (committed reg)
    #   self.rows = [Child(), Child()]  | "list"    | k.rows[0].<child attr>
    #   self.tag  = {"a": reg(8)}       | "dict"    | k.tag["a"].value
    #   anything else (int, fn, ...)    | None      | not in the manifest
    if isinstance(value, Module):
        return _module_node(value, path, namer, visiting)
    if isinstance(value, mem_blk):
        # BEFORE the slice test: mem_blk seeds an explicit data-width slice, so
        # `_is_user_sliced` is True even though it is a whole component.
        return _signal_node(value, namer)
    if isinstance(value, SignalRef):
        if value._is_user_sliced:                    # a slice VIEW — no net of its own
            # The emitted-name key IS the backend tag ("verilog": ...), so a
            # node can never be read with the wrong backend's name.
            return {"kind"        : "slice",
                    namer.backend : namer.hcp_name(value.ident),   # the PARENT signal's net
                    "msb"         : value._slice.stop - 1,         # DSL slices are exclusive-stop
                    "lsb"         : value._slice.start}
        return _signal_node(value, namer)
    if isinstance(value, Karray):
        return _karray_node(value, namer)
    if isinstance(value, counter):
        return {"kind" : "counter",
                "value": namer.hcp_name(value.value.ident),    # committed reg
                "now"  : namer.hcp_name(value.now.ident),      # add-chain head (unused by the reader today)
                "width": value.width}
    if isinstance(value, (list, tuple)):
        # WARNING: non-hardware items are dropped, so reader indices only match
        # the model's when the list is homogeneous hardware.
        items = [_attr_node(item, f"{path}[{n}]", namer, visiting) for n, item in enumerate(value)]
        items = [node for node in items if node is not None]
        return {"kind": "list", "items": items} if items else None
    if isinstance(value, dict) and all(isinstance(key, str) for key in value):
        entries: Dict[str, Any] = {}
        for key, item in value.items():
            node = _attr_node(item, f"{path}.{key}", namer, visiting)
            if node is not None:
                entries[key] = node
        return {"kind": "dict", "entries": entries} if entries else None
    return None                                        # ints, callables, KarrayRef access views, ...


def _signal_node(sig: SignalRef, namer: SimNamer) -> Dict[str, Any]:
    # The emitted-name key IS the backend tag ("verilog": ...) — see the slice node.
    return {"kind"        : "signal",
            namer.backend : namer.hcp_name(sig.ident),
            "hw_type"     : sig.ident.hw_type,
            "width"       : sig._slice.stop,           # seeded full slice = (0, width)
            "clocked"     : sig.ident.clocked}


def _karray_node(karray: Karray, namer: SimNamer) -> Dict[str, Any]:
    arena  = _session.arena()
    shape  = arena.karray_shape(karray.ident)
    fields = arena.karray_fields(karray.ident)
    elements = [
        {name: namer.hcp_name(arena.karray_element_hcp(karray.ident, list(coord), name))
         for name, _width in fields}
        for coord in product(*(range(dim) for dim in shape))   # row-major, reader indexes the same way
    ]
    return {"kind": "karray", "shape": list(shape),
            "fields": [[name, width] for name, width in fields],
            "elements": elements}


def _module_node(module: Module, path: str, namer: SimNamer, visiting: set) -> Dict[str, Any]:
    # `visiting` holds id()s along the CURRENT path only — the same sub-module
    # under two different attributes is fine (two valid entries), a loop is not.
    if id(module) in visiting:
        raise ValueError(f"sim manifest: module attribute cycle at {path!r}")
    visiting.add(id(module))
    # vars(module) is the instance __dict__: every `self.x = ...` an @init made.
    # The attribute names the user typed ARE the manifest keys — that is what
    # makes sim assist seamless (no per-signal registration call).
    children: Dict[str, Any] = {}
    for name, value in vars(module).items():
        if name.startswith("_"):                        # _ident + reader-internal names
            continue
        node = _attr_node(value, f"{path}.{name}", namer, visiting)
        if node is not None:                            # None = not a hardware attribute
            children[name] = node
    visiting.remove(id(module))
    return {"kind"    : "module",
            # top has no hierarchy hop to resolve — cocotb hands it in as `dut`;
            # every other instance name is the reader's getattr step.
            "instance": None if path == "top" else namer.module_name(module.ident),
            "children": children}


# ---- public entry ------------------------------------------------------------

def harvest_sim_tree(module: Module, namer: SimNamer) -> Dict[str, Any]:
    # The full manifest for one top module; root "instance" is None (it IS `dut`).
    # Example output (verilog backend, model from the _attr_node table):
    #
    #   {"schema_version": 1,
    #    "backend"       : "verilog",
    #    "top_module"    : "MODULE_Top0_0",
    #    "root": {"kind": "module", "instance": null, "children": {
    #      "x"  : {"kind": "signal" , "verilog" : "REG_x_1"    , "hw_type": "REG", "width": 8, "clocked": true},
    #      "hi" : {"kind": "slice"  , "verilog" : "REG_x_1"    , "msb"    : 7    , "lsb"  : 4},
    #      "cnt": {"kind": "counter", "value"   : "REG_c_CNT_2", "now"    : "WIRE_...", "width": 8},
    #      "rf" : {"kind": "karray" , "shape"   : [2]          , "fields" : [["data", 8]],
    #              "elements": [{"data": "REG_rf_E0_data_5"},
    #                           {"data": "REG_rf_E1_data_6"}]},
    #      "sub": {"kind": "module" , "instance": "MODULE_Child0_15",
    #              "children": {"acc": {"kind": "signal", "verilog": "REG_acc_16",
    #                                   "hw_type": "REG", "width": 8, "clocked": true}}}}}}
    return {"schema_version": 1,
            "backend"       : namer.backend,
            "top_module"    : namer.module_name(module.ident),
            "root"          : _module_node(module, "top", namer, set())}


def write_sim_manifest(module: Module, output_dir: str, backend: str) -> None:
    # Emit-time entry: pick the emitting backend's namer and drop the manifest
    # next to that backend's generated HDL (a KeyError = backend has no namer yet).
    namer = SIM_NAMERS[backend]()
    with open(os.path.join(output_dir, SIM_MANIFEST_FILE), "w", encoding="utf-8") as f:
        json.dump(harvest_sim_tree(module, namer), f, indent=1)
