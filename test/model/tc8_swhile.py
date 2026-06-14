# tc8 — sequential while: two explicit x=0 resets then x incremented while x < 3.
# swhile samples condition sequentially (one extra clock per iteration vs cwhile).

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc8_swhile"


# ---- model -------------------------------------------------------------------
class tc8_swhile(Module):
    @init
    def com_declare(self):
        self.x     = reg(8, "x")
        self.zero  = val(8, 0, "zero")
        self.limit = val(8, 3, "limit")
        self.one   = val(8, 1, "one")

        self.x.mark_output("my_x")

    @flow
    def my_flow(self):
        with seq():
            self.x |= self.zero
            self.x |= self.zero
            with swhile(self.x < self.limit):
                self.x |= self.x + self.one


# ---- build -------------------------------------------------------------------
def build(output_folder: str) -> None:
    reset()
    module = tc8_swhile()
    build_model(module)
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
@cocotb.test()
async def check_swhile(dut):
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value = 1
    await RisingEdge(dut.clk)            # E1
    await RisingEdge(dut.clk)            # E2
    await Timer(1, unit="ns")
    dut.mrst.value = 0

    # E3-E5: seq advances through two x<=0 resets; x stays 0.
    for _ in range(3):
        await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert dut.my_x.value == 0, f"my_x = {dut.my_x.value!s}"

    # swhile: condition sampled sequentially → 2 clocks per iteration.
    for i in range(1, 5):
        await RisingEdge(dut.clk)
        await RisingEdge(dut.clk)
        await Timer(1, unit="ns")
        assert dut.my_x.value == i, f"my_x = {dut.my_x.value!s} (expected {i})"


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
