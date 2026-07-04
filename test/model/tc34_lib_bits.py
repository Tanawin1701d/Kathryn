# tc34 — kathryn.lib.bits helpers: zext, sext, cat, replicate, or/and_reduce, mux.
# All comb: drive the inputs, check every derived output the same cycle.

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog
from kathryn.lib import zext, sext, cat, replicate, or_reduce, and_reduce, mux

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc34_lib_bits"


# ---- model -------------------------------------------------------------------
class tc34_lib_bits(Module):
    @init
    def com_declare(self):
        self.a    = wire(8, "a")
        self.b    = wire(4, "b")
        self.c    = wire(1, "c")

        self.o_zext = wire(8, "o_zext")
        self.o_sext = wire(8, "o_sext")
        self.o_cat  = wire(8, "o_cat")
        self.o_repl = wire(4, "o_repl")
        self.o_orr  = wire(1, "o_orr")
        self.o_andr = wire(1, "o_andr")
        self.o_mux  = wire(8, "o_mux")

        self.a.mark_input("a_in")
        self.b.mark_input("b_in")
        self.c.mark_input("c_in")
        for w, port in ((self.o_zext, "zext_out"), (self.o_sext, "sext_out"),
                        (self.o_cat,  "cat_out"),  (self.o_repl, "repl_out"),
                        (self.o_orr,  "orr_out"),  (self.o_andr, "andr_out"),
                        (self.o_mux,  "mux_out")):
            w.mark_output(port)

    @flow
    def my_flow(self):
        self.o_zext *= zext(self.b, 8)
        self.o_sext *= sext(self.b, 8)
        self.o_cat  *= cat(self.b, self.a[3, 0])          # {b, a[3:0]}
        self.o_repl *= replicate(self.c, 4)
        self.o_orr  *= or_reduce ([self.a[i] for i in range(8)])
        self.o_andr *= and_reduce([self.a[i] for i in range(8)])
        self.o_mux  *= mux(self.c, self.a, zext(self.b, 8))


# ---- build -------------------------------------------------------------------
def build(output_folder: str) -> None:
    reset()
    build_model(tc34_lib_bits())
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
def _model(a, b, c):
    sext_b = b | 0xF0 if b & 0x8 else b
    return {
        "zext_out": b,
        "sext_out": sext_b,
        "cat_out" : (b << 4) | (a & 0xF),
        "repl_out": 0xF if c else 0x0,
        "orr_out" : 1 if a != 0 else 0,
        "andr_out": 1 if a == 0xFF else 0,
        "mux_out" : a if c else b,
    }


@cocotb.test()
async def check_bits(dut):
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value = 1
    dut.a_in.value = 0
    dut.b_in.value = 0
    dut.c_in.value = 0
    await RisingEdge(dut.clk)
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    dut.mrst.value = 0

    vectors = [
        (0x00, 0x0, 0), (0xFF, 0xF, 1), (0xA5, 0x8, 0),   # 0x8: negative nibble
        (0x01, 0x7, 1), (0x80, 0x9, 0), (0x3C, 0x4, 1),
    ]
    for a, b, c in vectors:
        dut.a_in.value = a
        dut.b_in.value = b
        dut.c_in.value = c
        await Timer(2, unit="ns")
        for port, want in _model(a, b, c).items():
            got = int(getattr(dut, port).value)
            assert got == want, \
                f"a={a:#x} b={b:#x} c={c}: {port} = {got:#x} (expected {want:#x})"


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
