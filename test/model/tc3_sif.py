# tc3 — sequential if: x <= 42 only when cond_in is high.
# The condition is sampled sequentially (costs one extra clock vs cif).

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc3_sif"


# ---- model -------------------------------------------------------------------
class tc3_sif(Module):
    @init
    def com_declare(self):
        self.x       = reg(8, "x")
        self.cond_in = io_wire(1, True, "cond_in")   # 1-bit input
        self.val_42  = val(8, 42, "val_42")

        self.x.mark_output("my_x")

    @flow
    def my_flow(self):
        with seq():
            with sif(self.cond_in):
                self.x |= self.val_42


# ---- build -------------------------------------------------------------------
def build(output_folder: str) -> None:
    reset()
    module = tc3_sif()
    build_model(module)
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
@cocotb.test()
async def check_sif_taken(dut):
    # Branch taken: cond_in=1 after reset -> x should latch 42.
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value  = 1
    dut.cond_in.value = 1
    await RisingEdge(dut.clk)            # E1
    await RisingEdge(dut.clk)            # E2
    await Timer(1, unit="ns")
    dut.mrst.value = 0

    # E3: seq_state0 <= 1; sif condition sampled sequentially next cycle.
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert dut.my_x.value != 42, f"my_x latched too early: {dut.my_x.value!s}"

    # E4: sif condition result captured -> branch body active.
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert dut.my_x.value != 42, f"my_x latched too early (E4): {dut.my_x.value!s}"

    # E5: x <= 42 latched.
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert dut.my_x.value == 42, f"my_x = {dut.my_x.value!s} (expected 42)"


@cocotb.test()
async def check_sif_not_taken(dut):
    # Branch not taken: cond_in=0 -> x never becomes 42.
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value    = 1
    dut.cond_in.value = 0
    await RisingEdge(dut.clk)            # E1
    await RisingEdge(dut.clk)            # E2
    await Timer(1, unit="ns")
    dut.mrst.value = 0

    for _ in range(10):
        await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert dut.my_x.value != 42, f"my_x became 42 but cond_in was 0: {dut.my_x.value!s}"


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
