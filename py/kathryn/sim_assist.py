# Sim-assist READER — cocotb-side access to INTERNAL signals by the attribute
# names the model already uses (`k = KSim(dut); k.sub.x.value`), no
# mark_input/mark_output port needed. Counterpart: sim_manifest.py (writer).
# - Runs in the SIMULATOR subprocess: stdlib-only, never imports _kathryn,
#   cocotb, or the writer. Handles come in through `dut`.
# - Signal/counter lookups return the RAW cocotb handle, so `.value` works for
#   both observe and force/drive (a write is a VPI deposit).
# - All internals are `_`-prefixed; the manifest walker skips `_` attributes,
#   so a model attribute can never shadow the reader API.
# - Escape hatch: signals never stored on a Module attribute are not in the
#   manifest — reach them through the raw `dut` by their emitted name.
#
#  BUILD process (test runner)               SIM subprocess (cocotb coroutine)
#  ---------------------------               ---------------------------------
#  set_top(Top()) / build_model()            k = KSim(dut) ── loads ────┐
#  emit_verilog(out)                                                    │
#    └─ write_sim_manifest ───> out/sim_manifest.json  <────────────────┘
#         (sim_manifest.py)       ▲ path handed over via $KATHRYN_SIM_MANIFEST
#                                   (set per case by cocotb_pool/runner.py)
#
#  resolving `k.sub.rf[1].data.value = 9`, one manifest node per hop:
#    k       = KSimModule(dut, root children)
#    .sub  ─> "module"  node ─> KSimModule(getattr(dut, "MODULE_Child0_15"))
#    .rf   ─> "karray"  node ─> KSimKarray(handle, shape, elements)
#    [1]   ─> full rank      ─> KSimKarrayElement(elements[flat])   row-major
#    .data ─> field map      ─> getattr(handle, "REG_rf_E1_data_6") RAW handle
#    .value = 9              ─> VPI deposit (force); reads use the same handle

from __future__ import annotations

import json
import os
from typing import Any, Dict, List, Optional, Tuple, Union

SIM_MANIFEST_FILE = "sim_manifest.json"       # written next to the .v files
SIM_MANIFEST_ENV  = "KATHRYN_SIM_MANIFEST"    # set by the test runner for the sim subprocess

# LIMIT: cocotb handle types are unknowable here (cocotb is never imported), so
# handles and resolved children are typed Any.
_Handle = Any


# ---- node resolution ---------------------------------------------------------

def _resolve(node: Dict[str, Any], handle: _Handle, name_key: str) -> Any:
    # One manifest node -> its live sim-side object, dispatched on "kind".
    # `name_key` is the manifest's backend tag ("verilog", ...): the writer keys
    # each emitted name under the backend that produced it.
    kind = node["kind"]
    if kind == "module" : return KSimModule(getattr(handle, node["instance"]), node["children"], name_key)
    if kind == "signal" : return getattr(handle, node[name_key])
    if kind == "counter": return getattr(handle, node["value"])       # committed reg; "now" stays manifest-only
    if kind == "slice"  : return KSimSlice(getattr(handle, node[name_key]), node["msb"], node["lsb"])
    if kind == "karray" : return KSimKarray(handle, node["shape"], node["elements"])
    if kind == "list"   : return [_resolve(item, handle, name_key) for item in node["items"]]
    if kind == "dict"   : return {name: _resolve(sub, handle, name_key) for name, sub in node["entries"].items()}
    raise ValueError(f"sim manifest: unknown node kind {kind!r} (reader older than writer?)")


# ---- module view -------------------------------------------------------------

class KSimModule:
    """One module level of the sim tree: attribute access mirrors the model's
    own attribute names and resolves to cocotb handles / nested views."""
    def __init__(self, handle: _Handle, children: Dict[str, Any], name_key: str) -> None:
        self._handle   = handle
        self._children = children
        self._name_key = name_key                     # backend tag keying emitted names
        self._cache    : Dict[str, Any] = {}

    def __getattr__(self, name: str) -> Any:
        # Only reached when normal lookup fails, so `name` is a model attribute.
        if name.startswith("_"):
            raise AttributeError(name)
        if name not in self._cache:
            node = self._children.get(name)
            if node is None:
                raise AttributeError(
                    f"{name!r} is not in the sim manifest — only signals stored as "
                    f"Module attributes are visible (available: {sorted(self._children)})")
            self._cache[name] = _resolve(node, self._handle, self._name_key)
        return self._cache[name]

    def __dir__(self) -> List[str]:
        return sorted(self._children)


class KSim(KSimModule):
    """Root of the sim tree over `dut`. The manifest path defaults to the
    KATHRYN_SIM_MANIFEST env var (set by the test runner)."""
    def __init__(self, dut: _Handle, manifest_path: Optional[str] = None) -> None:
        path = manifest_path or os.environ.get(SIM_MANIFEST_ENV)
        if path is None:
            raise RuntimeError(
                f"no sim manifest: pass manifest_path or set ${SIM_MANIFEST_ENV} "
                f"(the cocotb_pool runner sets it automatically)")
        with open(path, "r", encoding="utf-8") as f:
            manifest = json.load(f)
        backend = manifest.get("backend", "verilog")   # which emitter produced the names
        super().__init__(dut, manifest["root"]["children"], backend)
        self._top_module = manifest["top_module"]      # emitted top module name, for reference
        self._backend    = backend


# ---- sliced view -------------------------------------------------------------

class KSimSlice:
    """Read-only window onto a whole signal's handle (a Python-side slice view
    has no net of its own in the Verilog)."""
    def __init__(self, handle: _Handle, msb: int, lsb: int) -> None:
        self._handle = handle
        self._msb    = msb
        self._lsb    = lsb

    @property
    def value(self) -> int:
        mask = (1 << (self._msb - self._lsb + 1)) - 1
        return (int(self._handle.value) >> self._lsb) & mask

    @value.setter
    def value(self, _v: int) -> None:
        raise AttributeError("a sliced view is read-only — force the whole signal instead")


# ---- karray view -------------------------------------------------------------

class KSimKarray:
    """Karray view: one int index per dimension (`k.rf[1]`, `k.rf[1][0]` or
    `k.rf[1, 0]`); a fully-indexed element exposes its fields as attributes."""
    def __init__(
        self,
        handle  : _Handle,
        shape   : List[int],
        elements: List[Dict[str, str]],        # row-major flat: field name -> verilog name
        fixed   : Tuple[int, ...] = (),
    ) -> None:
        self._handle   = handle
        self._shape    = shape
        self._elements = elements
        self._fixed    = fixed

    def __getitem__(self, key: Union[int, Tuple[int, ...]]) -> Any:
        idxs = key if isinstance(key, tuple) else (key,)
        for pos, idx in enumerate(idxs):
            dim = len(self._fixed) + pos
            if not isinstance(idx, int) or isinstance(idx, bool):
                raise TypeError("KSim karray indices must be ints")
            if dim >= len(self._shape):
                raise IndexError(f"too many indices for a {len(self._shape)}-D karray")
            if not 0 <= idx < self._shape[dim]:
                raise IndexError(f"index {idx} out of range for dim {dim} (size {self._shape[dim]})")
        fixed = self._fixed + tuple(idxs)
        if len(fixed) < len(self._shape):
            return KSimKarray(self._handle, self._shape, self._elements, fixed)
        flat = 0
        for size, idx in zip(self._shape, fixed):        # row-major, matches the writer's ordering
            flat = flat * size + idx
        return KSimKarrayElement(self._handle, self._elements[flat])

    def __len__(self) -> int:
        return self._shape[len(self._fixed)]


class KSimKarrayElement:
    """One fully-indexed karray element; each field attribute is the raw cocotb
    handle of that field's backing HCP (observe and force both work)."""
    def __init__(self, handle: _Handle, fields: Dict[str, str]) -> None:
        self._handle = handle
        self._fields = fields

    def __getattr__(self, name: str) -> Any:
        if name.startswith("_"):
            raise AttributeError(name)
        verilog = self._fields.get(name)
        if verilog is None:
            raise AttributeError(f"karray element has no field {name!r} (fields: {sorted(self._fields)})")
        return getattr(self._handle, verilog)

    def __dir__(self) -> List[str]:
        return sorted(self._fields)
