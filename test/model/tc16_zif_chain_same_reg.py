# tc16 — zero-cycle if/elif/else (zif / zelif / zelse) chain writing the SAME
# register three different values. The chain lowers to one clocked priority
# mux on REG x:
#     if (c1)      x <= 5
#     else if (c2) x <= 10
#     else         x <= 15
# Exactly one branch wins per cycle (mutually exclusive), so x latches the value
# of the first matching condition and holds it (nothing else drives x).

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog
from kathryn.sim_assist import KSim

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc16_zif_chain_same_reg"


# ---- model -------------------------------------------------------------------
class tc16_zif_chain_same_reg(Module):
    @init
    def com_declare(self):
        self.x      = reg (8, "x")
        self.c1     = wire(1, "c1")
        self.c2     = wire(1, "c2")
        self.val_5  = val (8, 5,  "val_5")
        self.val_10 = val (8, 10, "val_10")
        self.val_15 = val (8, 15, "val_15")

        self.c1.mark_input("c1_in")
        self.c2.mark_input("c2_in")

    @flow
    def my_flow(self):
        with seq():
            # Same register x, a different value down each arm of the chain.
            with zif(self.c1):
                self.x |= self.val_5        # x <= 5   when c1
            with zelif(self.c2):
                self.x |= self.val_10       # x <= 10  when !c1 & c2
            with zelse():
                self.x |= self.val_15       # x <= 15  otherwise


# ---- build -------------------------------------------------------------------
def build(output_folder: str) -> None:
    reset()
    module = tc16_zif_chain_same_reg()
    build_model(module)
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
async def _reset_then_hold(dut, c1: int, c2: int):
    # Drive the conditions, run two master-reset cycles, then release. Conditions
    # are held stable across the latch so x settles deterministically.
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value = 1
    dut.c1_in.value = c1
    dut.c2_in.value = c2
    await RisingEdge(dut.clk)            # E1
    await RisingEdge(dut.clk)            # E2
    await Timer(1, unit="ns")
    dut.mrst.value = 0

    # Let start -> seq_state propagate and the chain latch x; x then holds.
    for _ in range(5):
        await RisingEdge(dut.clk)
    await Timer(1, unit="ns")


@cocotb.test()
async def check_zif_branch(dut):
    # c1=1 → first arm wins → x = 5.
    k = KSim(dut)
    await _reset_then_hold(dut, c1=1, c2=0)
    assert k.x.value == 5, f"my_x = {k.x.value!s} (expected 5 — zif arm)"


@cocotb.test()
async def check_zelif_branch(dut):
    # c1=0, c2=1 → second arm wins → x = 10.
    k = KSim(dut)
    await _reset_then_hold(dut, c1=0, c2=1)
    assert k.x.value == 10, f"my_x = {k.x.value!s} (expected 10 — zelif arm)"


@cocotb.test()
async def check_zelse_branch(dut):
    # c1=0, c2=0 → else arm wins → x = 15.
    k = KSim(dut)
    await _reset_then_hold(dut, c1=0, c2=0)
    assert k.x.value == 15, f"my_x = {k.x.value!s} (expected 15 — zelse arm)"


@cocotb.test()
async def check_zif_priority_over_zelif(dut):
    # c1=1 AND c2=1 → the chain is a priority mux, so the zif arm still wins → x = 5.
    k = KSim(dut)
    await _reset_then_hold(dut, c1=1, c2=1)
    assert k.x.value == 5, f"my_x = {k.x.value!s} (expected 5 — zif outranks zelif)"


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
