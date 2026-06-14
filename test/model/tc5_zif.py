# tc5 — zero-cycle if: wires x and y driven combinationally based on conditions.
# zif holds no state — outputs reflect src values the very cycle conditions go high.

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc5_zif"


# ---- model -------------------------------------------------------------------
class tc5_zif(Module):
    @init
    def com_declare(self):
        self.x         = wire(8, "x")
        self.y         = wire(8, "y")
        self.cond_in   = wire(1, "cond")
        self.cond_in2  = wire(1, "cond_in2")
        self.src_val   = val (8, 24, "src_val")
        self.src_val2  = val (8, 48, "src_val2")

        self.cond_in .mark_input("cond_in")
        self.cond_in2.mark_input("cond_in2")

        self.x.mark_output("my_x")
        self.y.mark_output("my_y")

    @flow
    def my_flow(self):
        with seq():
            with zif(self.cond_in):
                self.x *= self.src_val
            with zelif(self.cond_in2):
                self.x *= self.src_val2
                self.y *= self.src_val2


# ---- build -------------------------------------------------------------------
def build(output_folder: str) -> None:
    reset()
    module = tc5_zif()
    build_model(module)
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
@cocotb.test()
async def check_zif_taken(dut):
    # zif branch: cond_in=1 -> x reflects 24 combinationally; drop -> x stops.
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value     = 1
    dut.cond_in .value = 1
    dut.cond_in2.value = 0
    await RisingEdge(dut.clk)            # E1
    await RisingEdge(dut.clk)            # E2
    await Timer(1, unit="ns")
    dut.mrst.value = 0

    # Settle past start state with no conditions active.
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert dut.my_x.value == 24, f"my_x is not set to correct value: {dut.my_x.value!s}"
    assert dut.my_y.value != 48, f"my_y should not be set"
    await RisingEdge(dut.clk)


@cocotb.test()
async def check_zif_not_taken(dut):
    # Neither branch active: outputs stay at default (0) the whole time.
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value     = 1
    dut.cond_in .value = 0
    dut.cond_in2.value = 0
    await RisingEdge(dut.clk)
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    dut.mrst.value = 0

    # Settle past start state with no conditions active.
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert dut.my_x.value != 24, f"my_x should not be set"
    assert dut.my_y.value != 48, f"my_y should not be set"
    await RisingEdge(dut.clk)



@cocotb.test()
async def check_zelif_taken(dut):
    # zelif branch: cond_in=0, cond_in2=1 -> x and y reflect 48 combinationally.
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value     = 1
    dut.cond_in .value = 0
    dut.cond_in2.value = 1
    await RisingEdge(dut.clk)            # E1
    await RisingEdge(dut.clk)            # E2
    await Timer(1, unit="ns")
    dut.mrst.value = 0

    # Settle past start state with no conditions active.
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert dut.my_x.value == 48, f"my_x is not set to correct value: {dut.my_x.value!s}"
    assert dut.my_y.value == 48, f"my_y should not be set"
    await RisingEdge(dut.clk)




# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
