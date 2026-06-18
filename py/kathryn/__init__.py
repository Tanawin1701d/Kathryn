# Public Kathryn DSL. `from kathryn import *` brings in every factory, the signal
# base/expression types, the flow-block and module scopes, and the LogicOp enum.
# Importing this package builds the one ModelArena (see _session); ownership of
# all model objects stays in Rust and is reached only through opaque idents.

from . import _session                                          # creates the singleton arena
from ._kathryn import LogicOp, FlowBlockType, Slice, HcpIdent, FlowBlockIdent, ModuleIdent, BackendVerilog
from ._session import arena, reset, gen_flow, build_flow, build_model, emit_verilog
from .signal import SignalRef, expr
from .hw_component import reg, wire, val, mem_blk, mem_ele
from .flow_block import (
    seq, par, par_auto, par_no_sync,
    cif, sif, cselif, cselse,
    zif, zelif, zelse,
    zstate, zcase,
    cwhile, swhile, cdowhile, cloop,
    scwait, sywait,
    pip,
    zync,
)
from .module import Module, init, flow, set_top
from . import priority as _priority
from .priority import (
    priority, set_priority, set_priority_auto, get_priority, get_priority_mode,
)
# Re-export every UE-priority constant by the name list the Rust source publishes,
# so the constants stay in lockstep with the host with nothing hardcoded here.
globals().update({n: getattr(_priority, n) for n in _priority.PRIORITY_CONST_NAMES})

__all__ = [
    # rust enums / idents
    "LogicOp", "FlowBlockType", "Slice", "HcpIdent", "FlowBlockIdent", "ModuleIdent",
    # backends
    "BackendVerilog",
    # signals
    "SignalRef", "expr",
    "reg", "wire", "val", "mem_blk", "mem_ele",
    # flow blocks
    "seq", "par", "par_auto", "par_no_sync",
    "cif", "sif", "cselif", "cselse",
    "zif", "zelif", "zelse",
    "zstate", "zcase",
    "cwhile", "swhile", "cdowhile", "cloop",
    "scwait", "sywait",
    "pip",
    "zync",
    # module scope + session
    "Module", "init", "flow", "arena", "reset",
    "gen_flow", "build_flow", "build_model", "set_top", "emit_verilog",
    # asm-node priority
    "priority", "set_priority", "set_priority_auto", "get_priority", "get_priority_mode",
    *_priority.PRIORITY_CONST_NAMES,
]
