# tc22 — pick block: a multi-way combinational select that is NOT mutex-chained.
#
# Unlike `cif`/`cselif` (which chain branches so at most one fires), a `pick` runs
# EVERY `pif` whose raw condition is high — the user is responsible for keeping the
# conditions mutually exclusive. The optional `pidef` default fires only when no
# `pif` matched (`~sel0 & ~sel1`). Each branch latches the shared output reg `r`:
#
#   pick:
#       pif(sel0):   seq[ r |= v_a   (11) ]
#       pif(sel1):   seq[ r |= v_b   (22) ]
#       pidef():     seq[ r |= v_def (33) ]
#
# With sel0/sel1 kept mutually exclusive the result is a clean 3-way mux into r:
#   sel0=1 -> r==11 ,  sel1=1 -> r==22 ,  neither -> r==33 (default).
# NOTE: the pick exit is NOT auto-synchronized — the matching branch drives it.

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc22_pick"

V_A   = 11                              # pif(sel0) writes this
V_B   = 22                              # pif(sel1) writes this
V_DEF = 33                              # pidef default writes this

SETTLE = 2                              # cycles to let the picked branch latch r


# ---- model -------------------------------------------------------------------
class tc22_pick(Module):
    @init
    def com_declare(self):
        self.r     = reg (8, "r")       # shared output, driven by the picked branch
        self.sel0  = wire(1, "sel0")    # selects branch a (v_a)
        self.sel1  = wire(1, "sel1")    # selects branch b (v_b)
        self.v_a   = val (8, V_A,   "v_a")
        self.v_b   = val (8, V_B,   "v_b")
        self.v_def = val (8, V_DEF, "v_def")

        self.sel0.mark_input("sel0")
        self.sel1.mark_input("sel1")

        self.r.mark_output("my_r")

    @flow
    def my_flow(self):
        self.r.reset(0)

        # Three-way pick: each pif gated on its own raw select; pidef catches "none".
        with pick():
            with pif(self.sel0):
                self.r |= self.v_a
            with pif(self.sel1):
                self.r |= self.v_b
            with pidef():
                self.r |= self.v_def


# ---- build (kathryn model -> verilog) ---------------------------------------
def build(output_folder: str) -> None:
    reset()
    module = tc22_pick()
    build_model(module)
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
async def _reset_with_sel(dut, sel0, sel1):
    # Drive the clock, hold master reset for two edges (r loads 0, start arms) with
    # the selects already stable, then release reset and let the pick settle.
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())
    dut.mrst.value = 1
    dut.sel0.value = sel0
    dut.sel1.value = sel1
    await RisingEdge(dut.clk)            # E1
    await RisingEdge(dut.clk)            # E2
    await Timer(1, unit="ns")
    dut.mrst.value = 0

    for _ in range(SETTLE):
        await RisingEdge(dut.clk)
    await Timer(1, unit="ns")


@cocotb.test()
async def check_pif0_selected(dut):
    # sel0 high, sel1 low -> only branch a fires, so r latches v_a.
    await _reset_with_sel(dut, 1, 0)
    assert dut.my_r.value == V_A, f"my_r = {dut.my_r.value!s} (expected {V_A})"


@cocotb.test()
async def check_pif1_selected(dut):
    # sel1 high, sel0 low -> only branch b fires, so r latches v_b.
    await _reset_with_sel(dut, 0, 1)
    assert dut.my_r.value == V_B, f"my_r = {dut.my_r.value!s} (expected {V_B})"


@cocotb.test()
async def check_pidef_default(dut):
    # Neither select high -> no pif matched, so the pidef default drives r to v_def.
    await _reset_with_sel(dut, 0, 0)
    assert dut.my_r.value == V_DEF, f"my_r = {dut.my_r.value!s} (expected {V_DEF})"


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
