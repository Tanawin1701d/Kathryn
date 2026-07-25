# tc43 — kathryn.lib selection helpers: muxn (N:1), decoder (bin→one-hot),
# priority_encoder (lowest-set index + valid).
# All comb: drive the inputs, check every derived output the same cycle.

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog
from kathryn.lib import muxn, decoder, priority_encoder, cat

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc43_lib_select"


# ---- model -------------------------------------------------------------------
class tc43_lib_select(Module):
    @init
    def com_declare(self):
        self.sel   = wire(2, "sel")
        self.flags = wire(5, "flags")
        self.d0    = wire(8, "d0")
        self.d1    = wire(8, "d1")
        self.d2    = wire(8, "d2")

        self.o_mux  = wire(8, "o_mux")
        self.o_dec  = wire(4, "o_dec")
        self.o_idx  = wire(3, "o_idx")
        self.o_vld  = wire(1, "o_vld")

        self.sel  .mark_input("sel_in")
        self.flags.mark_input("flags_in")
        self.d0   .mark_input("d0_in")
        self.d1   .mark_input("d1_in")
        self.d2   .mark_input("d2_in")
        for w, port in ((self.o_mux, "mux_out"), (self.o_dec, "dec_out"),
                        (self.o_idx, "idx_out"), (self.o_vld, "vld_out")):
            w.mark_output(port)

    @flow
    def my_flow(self):
        # sel 0/1/2 pick d0/d1/d2; sel==3 falls into the zelse arm (d2).
        self.o_mux *= muxn(self.sel, [self.d0, self.d1, self.d2])
        self.o_dec *= cat(*reversed(decoder(self.sel, 4)))      # bit i = (sel == i)
        idx, vld    = priority_encoder([self.flags[i] for i in range(5)])
        self.o_idx *= idx
        self.o_vld *= vld


# ---- build -------------------------------------------------------------------
def build(output_folder: str) -> None:
    reset()
    build_model(tc43_lib_select())
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
def _model(sel, flags, d0, d1, d2):
    lowest = next((i for i in range(5) if (flags >> i) & 1), 0)
    return {
        "mux_out": [d0, d1, d2, d2][sel],
        "dec_out": (1 << sel) & 0xF,
        "idx_out": lowest,
        "vld_out": 1 if flags else 0,
    }


@cocotb.test()
async def check_select(dut):
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value     = 1
    dut.sel_in.value   = 0
    dut.flags_in.value = 0
    dut.d0_in.value    = 0
    dut.d1_in.value    = 0
    dut.d2_in.value    = 0
    await RisingEdge(dut.clk)
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    dut.mrst.value = 0

    vectors = [                                    # (sel, flags, d0, d1, d2)
        (0, 0b00000, 0x11, 0x22, 0x33), (1, 0b00001, 0xAA, 0xBB, 0xCC),
        (2, 0b10000, 0x01, 0x02, 0x03), (3, 0b01010, 0xDE, 0xAD, 0xBF),
        (1, 0b11111, 0x55, 0x66, 0x77), (0, 0b00100, 0xF0, 0x0F, 0xFF),
    ]
    for sel, flags, d0, d1, d2 in vectors:
        dut.sel_in.value   = sel
        dut.flags_in.value = flags
        dut.d0_in.value    = d0
        dut.d1_in.value    = d1
        dut.d2_in.value    = d2
        await Timer(2, unit="ns")
        for port, want in _model(sel, flags, d0, d1, d2).items():
            got = int(getattr(dut, port).value)
            assert got == want, \
                f"sel={sel} flags={flags:#07b}: {port} = {got:#x} (expected {want:#x})"


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
