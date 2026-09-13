# cocotb — the cocotb harness: which simulator cocotb's runner drives, and the
# flags it needs.  One file per tool.
#
#   base.py        CocotbBackend: make_runner per tool; compile / run shared; COCOTB_HDL_OF
#   verilator.py   the default: wheel / conda discovery, the PCH repair, -Wno-fatal
#   icarus.py      the switch: plain VCD where cocotb would write FST
#
# The switch itself (BACKENDS, get_backend) lives here because this is the one
# place that knows every tool this harness drives; a new one is a new file plus
# one row.

from .base      import COCOTB_HDL_OF, CocotbBackend, TIMESCALE, cocotb_hdl_of, cocotb_is_available
from .icarus    import IcarusBackend, icarus_is_available, make_icarus_runner
from .verilator import (VERILATOR_CONDA, VERILATOR_MIN, VerilatorBackend, find_conda_verilator,
                        find_wheel_verilator, is_new_enough, make_verilator_runner,
                        parse_verilator_version, put_verilator_on_path, read_verilator_version,
                        verilator_is_available)

BACKENDS = {"verilator": VerilatorBackend,
            "icarus"   : IcarusBackend}


def get_backend(name: str) -> CocotbBackend:
    if name not in BACKENDS:
        raise ValueError(f"unknown simulator '{name}' — one of {sorted(BACKENDS)}")
    return BACKENDS[name]()


__all__ = ["CocotbBackend"          , "cocotb_is_available"   ,
           "COCOTB_HDL_OF"          , "cocotb_hdl_of"         , "TIMESCALE"            ,
           "BACKENDS"               , "get_backend"           ,
           "IcarusBackend"          , "icarus_is_available"   , "make_icarus_runner"   ,
           "VerilatorBackend"       , "verilator_is_available", "make_verilator_runner",
           "VERILATOR_MIN"          , "VERILATOR_CONDA"       , "put_verilator_on_path",
           "find_wheel_verilator"   , "find_conda_verilator"  , "is_new_enough"        ,
           "parse_verilator_version", "read_verilator_version"]
