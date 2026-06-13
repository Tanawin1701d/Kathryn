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
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
@cocotb.test()
async def check_seq(dut):
    # 10ns clock; assert master-reset for two cycles to launch the sequence.
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value = 1
    await RisingEdge(dut.clk)            # E1: start <= 1 (s0 held 0 by mrst)
    await RisingEdge(dut.clk)            # E2: start stays 1 (s0 still held 0 by mrst)
    await Timer(1, unit="ns")
    dut.mrst.value = 0

    # Sequence after mrst deasserts: E3 sets seq_state0, E4 latches x, E5 latches y.
    # x/y have no reset, so they read X (uninitialized) until first latched —
    # use `!= 48` (X-safe) for the "not yet" guards, not `== 0`.
    await RisingEdge(dut.clk)            # E3: seq_state0 <= 1
    await Timer(1, unit="ns")
    assert dut.my_x.value != 48, f"my_x latched too early = {dut.my_x.value!s}"
    assert dut.my_y.value != 48, f"my_y latched too early = {dut.my_y.value!s}"

    await RisingEdge(dut.clk)            # E4: x <= 48 — first cycle my_x is asserted
    await Timer(1, unit="ns")
    assert dut.my_x.value == 48, f"my_x at first assert = {dut.my_x.value!s} (expected 48)"
    assert dut.my_y.value != 48, f"my_y latched too early = {dut.my_y.value!s}"

    await RisingEdge(dut.clk)            # E5: y <= x — first cycle my_y is asserted
    await Timer(1, unit="ns")
    assert dut.my_y.value == 48, f"my_y at first assert = {dut.my_y.value!s} (expected 48)"
    assert dut.my_x.value == 48, f"my_x held = {dut.my_x.value!s} (expected 48)"


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
