# tc4 — combinational if: same as tc3 but condition is sampled combinationally
# (zero extra cycles for condition check), so x latches one cycle earlier than sif.

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc4_cif"


# ---- model -------------------------------------------------------------------
class tc4_cif(Module):
    @init
    def com_declare(self):
        self.x       = reg(8, "x")
        self.cond_in = io_wire(1, True, "cond_in")
        self.val_42  = val(8, 42, "val_42")

        self.x.mark_output("my_x")

    @flow
    def my_flow(self):
        with seq():
            with cif(self.cond_in):
                self.x |= self.val_42


# ---- build -------------------------------------------------------------------
def build(output_folder: str) -> None:
    reset()
    module = tc4_cif()
    build_model(module)
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
@cocotb.test()
async def check_cif_taken(dut):
    # cif samples the condition combinationally — one cycle earlier than sif.
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value    = 1
    dut.cond_in.value = 1
    await RisingEdge(dut.clk)            # E1
    await RisingEdge(dut.clk)            # E2
    await Timer(1, unit="ns")
    dut.mrst.value = 0

    # E3: seq_state0 <= 1; cif condition is combinational so body active same cycle.
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert dut.my_x.value != 42, f"my_x latched too early: {dut.my_x.value!s}"

    # E4: x <= 42 latched (one cycle earlier than sif would).
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert dut.my_x.value == 42, f"my_x = {dut.my_x.value!s} (expected 42)"


@cocotb.test()
async def check_cif_not_taken(dut):
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value    = 1
    dut.cond_in.value = 0
    await RisingEdge(dut.clk)
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    dut.mrst.value = 0

    for _ in range(10):
        await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert dut.my_x.value != 42, f"my_x became 42 but cond_in was 0: {dut.my_x.value!s}"


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
