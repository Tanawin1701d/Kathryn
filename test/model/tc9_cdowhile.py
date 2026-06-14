# tc9 — do-while: body runs at least once then repeats while x < 3.
# Even if x starts at 3, one iteration still fires (unlike cwhile/swhile).

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc9_cdowhile"


# ---- model -------------------------------------------------------------------
class tc9_cdowhile(Module):
    @init
    def com_declare(self):
        self.x     = reg(8, "x")
        self.limit = val(8, 3, "limit")
        self.one   = val(8, 1, "one")

        self.x.mark_output("my_x")

    @flow
    def my_flow(self):
        with seq():
            with cdowhile(self.x < self.limit):
                self.x |= self.x + self.one


# ---- build -------------------------------------------------------------------
def build(output_folder: str) -> None:
    reset()
    module = tc9_cdowhile()
    build_model(module)
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
@cocotb.test()
async def check_cdowhile(dut):
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value = 1
    await RisingEdge(dut.clk)            # E1
    await RisingEdge(dut.clk)            # E2
    await Timer(1, unit="ns")
    dut.mrst.value = 0

    # Body fires at least once; 3 iterations bring x from 0 to 3, then loop exits.
    for _ in range(12):
        await RisingEdge(dut.clk)
    await Timer(1, unit="ns")

    assert dut.my_x.value == 4, f"my_x = {dut.my_x.value!s} (expected 4)"


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
