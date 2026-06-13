# tc2 — parallel auto-sync: x and y assigned in two branches, both settle same cycle.

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc2_par"


# ---- model -------------------------------------------------------------------
class tc2_par(Module):
    @init
    def com_declare(self):
        self.x     = reg(8, "x")
        self.y     = reg(8, "y")
        self.val_5  = val(8, 5,  "val_5")
        self.val_10 = val(8, 10, "val_10")

        self.x.mark_output("my_x")
        self.y.mark_output("my_y")

    @flow
    def my_flow(self):
        with seq():
            with par_auto():
                self.x |= self.val_5
                self.y |= self.val_10


# ---- build -------------------------------------------------------------------
def build(output_folder: str) -> None:
    reset()
    module = tc2_par()
    build_model(module)
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
@cocotb.test()
async def check_par(dut):
    # 10ns clock; two cycles of master-reset.
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value = 1
    await RisingEdge(dut.clk)            # E1
    await RisingEdge(dut.clk)            # E2
    await Timer(1, unit="ns")
    dut.mrst.value = 0

    # E3: seq_state0 <= 1  (start -> par_auto body entered next cycle)
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert dut.my_x.value != 5,  f"my_x latched too early: {dut.my_x.value!s}"
    assert dut.my_y.value != 10, f"my_y latched too early: {dut.my_y.value!s}"

    # E4: both branches latch on the same cycle (auto-sync).
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert dut.my_x.value == 5,  f"my_x = {dut.my_x.value!s} (expected 5)"
    assert dut.my_y.value == 10, f"my_y = {dut.my_y.value!s} (expected 10)"

    for i in range(10):
        await RisingEdge(dut.clk)
    await Timer(1, unit="ns")


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
