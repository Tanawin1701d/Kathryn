# Public Kathryn DSL. `from kathryn import *` brings in every factory, the signal
# base/expression types, the flow-block and module scopes, and the LogicOp enum.
# Importing this package builds the one ModelArena (see _session); ownership of
# all model objects stays in Rust and is reached only through opaque idents.

from . import _session                                          # creates the singleton arena
from ._kathryn import LogicOp, Slice, HcpIdent, FlowBlockIdent, ModuleIdent
from ._session import arena, reset
from .signal import SignalRef, expr
from .hw_component import reg, wire, val, io_wire, mem_blk, mem_ele
from .flow_block import (
    seq, par_auto, par_no_sync,
    cif, sif, cselif, cselse,
    zif, zelif, zelse,
    cwhile, swhile, do_while, counter_loop,
)
from .module import module

__all__ = [
    # rust enums / idents
    "LogicOp", "Slice", "HcpIdent", "FlowBlockIdent", "ModuleIdent",
    # signals
    "SignalRef", "expr",
    "reg", "wire", "val", "io_wire", "mem_blk", "mem_ele",
    # flow blocks
    "seq", "par_auto", "par_no_sync",
    "cif", "sif", "cselif", "cselse",
    "zif", "zelif", "zelse",
    "cwhile", "swhile", "do_while", "counter_loop",
    # module scope + session
    "module", "arena", "reset",
]
