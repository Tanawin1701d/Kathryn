# tc7 — combinational while: x incremented each iteration while x < 3.
# Condition is sampled combinationally (zero extra cycles per check).

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc7_cwhile"


# ---- model -------------------------------------------------------------------
class tc7_cwhile(Module):
    @init
    def com_declare(self):
        self.x     = reg(8, "x")
        self.limit = val(8, 3, "limit")
        self.one   = val(8, 1, "one")

        self.x.mark_output("my_x")

    @flow
    def my_flow(self):
        with seq():
            with cwhile(self.x < self.limit):
                self.x |= self.x + self.one


# ---- build -------------------------------------------------------------------
def build(output_folder: str) -> None:
    reset()
    module = tc7_cwhile()
    build_model(module)
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
@cocotb.test()
async def check_cwhile(dut):
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value = 1
    await RisingEdge(dut.clk)            # E1
    await RisingEdge(dut.clk)            # E2
    await Timer(1, unit="ns")
    dut.mrst.value = 0

    # Give enough cycles for 3 iterations plus margin.
    for _ in range(12):
        await RisingEdge(dut.clk)
    await Timer(1, unit="ns")

    assert dut.my_x.value == 3, f"my_x = {dut.my_x.value!s} (expected 3)"


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
