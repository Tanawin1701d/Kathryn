# tc6 — counter loop: two explicit x=0 resets then x incremented 4 times.
# Seq order: (x<=0) -> (x<=0) -> cloop(3) body x4 → final x == 4.

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc6_cloop"


# ---- model -------------------------------------------------------------------
class tc6_cloop(Module):
    @init
    def com_declare(self):
        self.x    = reg(8, "x")
        self.one  = val(8, 1, "one")
        self.zero = val(8, 0, "zero")

        self.x.mark_output("my_x")

    @flow
    def my_flow(self):
        with seq():
            self.x |= self.zero
            self.x |= self.zero
            with cloop(3):    # 4 iterations (last_loop_cnt = 3)
                self.x |= self.x + self.one


# ---- build -------------------------------------------------------------------
def build(output_folder: str) -> None:
    reset()
    module = tc6_cloop()
    build_model(module)
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
@cocotb.test()
async def check_cloop(dut):
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value = 1
    await RisingEdge(dut.clk)            # E1
    await RisingEdge(dut.clk)            # E2
    await Timer(1, unit="ns")
    dut.mrst.value = 0

    # E3-E4: seq advances through the two x<=0 assignments; x stays 0.
    await RisingEdge(dut.clk)            # E3
    # first zero
    await RisingEdge(dut.clk)            # E4
    # second zero
    await RisingEdge(dut.clk)            # E4
    # loop state
    await RisingEdge(dut.clk)            # E4
    # loop state
    await Timer(1, unit="ns")
    assert dut.my_x.value == 1, f"my_x = {dut.my_x.value!s} (expected 1)"

    # E5-E8: 4 cloop iterations increment x from 0 to 4.
    # Give extra margin for loop overhead cycles.
    for _ in range(12):
        await RisingEdge(dut.clk)
    await Timer(1, unit="ns")

    assert dut.my_x.value == 3, f"my_x = {dut.my_x.value!s} (expected 4 after 4 iterations)"


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
