# tc4 — combinational if: same as tc3 but condition is sampled combinationally
# (zero extra cycles for condition check), so x latches one cycle earlier than sif.

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog
from kathryn.sim_assist import KSim

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc4_cif"


# ---- model -------------------------------------------------------------------
class tc4_cif(Module):
    @init
    def com_declare(self):
        self.x        = reg (8, "x")
        self.y        = reg (8, "y")
        self.cond_in  = wire(1, "cond")
        self.cond_in2 = wire(1, "cond_in2")
        self.val_42   = val (8, 42, "val_42")
        self.val_48   = val (8, 48, "val_48")

        self.cond_in .mark_input("cond_in")
        self.cond_in2.mark_input("cond_in2")

    @flow
    def my_flow(self):
        with seq():
            with cif(self.cond_in):
                self.x |= self.val_42
            with cselif(self.cond_in2):
                with par():
                    self.x |= self.val_48
                    self.y |= self.val_48


# ---- build -------------------------------------------------------------------
def build(output_folder: str) -> None:
    reset()
    module = tc4_cif()
    build_model(module)
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
@cocotb.test()
async def check_cif_taken(dut):
    k = KSim(dut)
    # cif samples the condition combinationally — one cycle earlier than sif.
    # Branch taken: cond_in=1 -> x latches 42 at E4 (vs E5 for sif).
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value    = 1
    dut.cond_in.value = 1
    await RisingEdge(dut.clk)            # E1
    await RisingEdge(dut.clk)            # E2
    await Timer(1, unit="ns")
    dut.mrst.value    = 0
    dut.cond_in .value = 1
    dut.cond_in2.value = 0

    # E3: seq_state0 <= 1; cif condition is combinational, body active same cycle.
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert k.x.value != 42, f"my_x latched too early: {k.x.value!s}"

    # E4: x <= 42 latched (one cycle earlier than sif).
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert k.x.value == 42, f"my_x = {k.x.value!s} (expected 42)"
    await RisingEdge(dut.clk)


@cocotb.test()
async def check_cif_not_taken(dut):
    k = KSim(dut)
    # Neither branch taken: cond_in=0, cond_in2=0 -> x stays at reset value.
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value    = 1
    dut.cond_in.value = 0
    await RisingEdge(dut.clk)
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    dut.mrst.value     = 0
    dut.cond_in .value = 0
    dut.cond_in2.value = 0

    for _ in range(10):
        await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert k.x.value != 42, f"my_x became 42 but cond_in was 0: {k.x.value!s}"
    assert k.x.value != 48, f"my_x became 48 but neither cond was set: {k.x.value!s}"


@cocotb.test()
async def check_elif_taken(dut):
    k = KSim(dut)
    # elif branch taken: cond_in=0, cond_in2=1 -> x and y latch 48 at E4.
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value     = 1
    dut.cond_in .value = 0
    dut.cond_in2.value = 1
    await RisingEdge(dut.clk)            # E1
    await RisingEdge(dut.clk)            # E2
    await Timer(1, unit="ns")
    dut.mrst.value     = 0
    dut.cond_in .value = 0
    dut.cond_in2.value = 1

    # E3: seq_state0 <= 1; elif condition combinational, par body active same cycle.
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert k.x.value != 48, f"my_x latched too early (E3): {k.x.value!s}"
    assert k.y.value != 48, f"my_y latched too early (E3): {k.y.value!s}"

    # E4: x and y <= 48 latched in parallel.
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert k.x.value == 48, f"my_x = {k.x.value!s} (expected 48)"
    assert k.y.value == 48, f"my_y = {k.y.value!s} (expected 48)"
    await RisingEdge(dut.clk)


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
