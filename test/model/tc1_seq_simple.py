# tc1 — a single sequential block: x <= simple_val, then y <= x.
#
# This file is self-contained: it (1) describes the kathryn model, (2) provides a
# `build` fn that emits Verilog, (3) holds the cocotb simulation, and (4)
# registers itself into the shared pool. `cocotb_pool.run_all()` drives it; there
# is no Makefile.

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc1_seq_simple"


# ---- model -------------------------------------------------------------------
class tc1_seq_simple(Module):
    @init
    def com_declare(self):
        self.x          = reg(8, "x")
        self.y          = reg(8, "y")
        self.simple_val = val(8, 48, "simple_val")

        self.x.mark_output("my_x")
        self.y.mark_output("my_y")

    @flow
    def my_flow(self):
        with seq():
            self.x |= self.simple_val
            self.y |= self.x


# ---- build (kathryn model -> verilog) ---------------------------------------
def build(output_folder: str) -> None:
    reset()
    module = tc1_seq_simple()
    build_model(module)
    #emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
@cocotb.test()
async def check_seq(dut):
    # 10ns clock; assert master-reset for two cycles to launch the sequence.
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value = 1
    await RisingEdge(dut.clk)
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    dut.mrst.value = 0

    # Sequence: start -> state0 (x<=48) -> state1 (y<=x). A handful of clocks
    # is plenty; give margin then check the outputs settled.
    for _ in range(8):
        await RisingEdge(dut.clk)
    await Timer(1, unit="ns")

    assert dut.my_x.value == 48, f"my_x = {int(dut.my_x.value)} (expected 48)"
    assert dut.my_y.value == 48, f"my_y = {int(dut.my_y.value)} (expected 48)"


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
