# tc5 — zero-cycle if: wire x driven combinationally when cond_in is high.
# zif holds no state — x reflects src_val the very cycle cond_in goes high.

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
        self.x        = wire(8, "x")
        self.cond_in  = io_wire(1, True,  "cond_in")
        self.src_val  = val(8, 99, "src_val")

        self.x.mark_output("my_x")

    @flow
    def my_flow(self):
        with seq():
            with zif(self.cond_in):
                self.x *= self.src_val   # *= is combinational assign for wire


# ---- build -------------------------------------------------------------------
def build(output_folder: str) -> None:
    reset()
    module = tc5_zif()
    build_model(module)
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
@cocotb.test()
async def check_zif(dut):
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value    = 1
    dut.cond_in.value = 0
    await RisingEdge(dut.clk)            # E1
    await RisingEdge(dut.clk)            # E2
    await Timer(1, unit="ns")
    dut.mrst.value = 0

    # Give the sequence a few cycles to settle past start state.
    for _ in range(4):
        await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert dut.my_x.value != 99, f"my_x driven while cond_in=0: {dut.my_x.value!s}"

    # Drive cond_in high; zif is combinational so x should reflect src_val
    # within the same delta, visible 1ns after the next rising edge.
    dut.cond_in.value = 1
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert dut.my_x.value == 99, f"my_x = {dut.my_x.value!s} (expected 99)"

    # Drop cond_in — x should stop being 99 immediately.
    dut.cond_in.value = 0
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert dut.my_x.value != 99, f"my_x still 99 after cond_in=0: {dut.my_x.value!s}"


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
