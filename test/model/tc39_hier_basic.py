# tc39 — basic module hierarchy: Top + one child, cross-module routing both ways.
#
# Covers:
#   * parent → child routing: child's comb wire and clocked reg read Top's input wire
#   * child → parent routing: Top's output wires read the child's wire/reg
#   * implicit clk / mrst forwarding into the child's clocked seq flow
#
# Child computes dbl = 2*src combinationally and latches acc = 2*src once at the
# post-reset start pulse (tc1-style one-shot seq), so a later src change moves
# dbl_out but must NOT move acc_out — proving acc is truly clocked off the
# forwarded clk, not a comb copy.

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc39_hier_basic"


# ---- model -------------------------------------------------------------------
class HierChild(Module):
    # `self.src` (a SignalRef living in the PARENT) is injected by the parent
    # after construction, before gen_flow() runs the deferred @flow methods.
    @init
    def com_declare(self):
        self.dbl = wire(8, "dbl")
        self.acc = reg (8, "acc")

    @flow
    def my_flow(self):
        self.dbl.default(self.src + self.src)   # comb read of a parent signal
        self.acc.reset(0)
        with seq():
            self.acc |= self.dbl                # clocked — needs forwarded clk


class tc39_hier_basic(Module):
    @init
    def com_declare(self):
        self.src = wire(8, "src")
        self.src.mark_input("src_in")

        self.child     = HierChild("child")
        self.child.src = self.src               # parent → child signal handoff

        self.out_dbl = wire(8, "out_dbl")
        self.out_acc = wire(8, "out_acc")
        self.out_dbl.mark_output("dbl_out")
        self.out_acc.mark_output("acc_out")

    @flow
    def my_flow(self):
        self.out_dbl.default(self.child.dbl)    # child → parent comb reads
        self.out_acc.default(self.child.acc)


# ---- build -------------------------------------------------------------------
def build(output_folder: str) -> None:
    reset()
    module = tc39_hier_basic()
    build_model(module)
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
@cocotb.test()
async def check_hier_routing(dut):
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value   = 1
    dut.src_in.value = 21
    await RisingEdge(dut.clk)            # E1
    await RisingEdge(dut.clk)            # E2
    await Timer(1, unit="ns")
    dut.mrst.value = 0

    # Comb path Top.src -> child.dbl -> Top.out_dbl is live regardless of the
    # start sequence.
    assert dut.dbl_out.value == 42, f"dbl_out = {dut.dbl_out.value!s} (expected 42)"

    await RisingEdge(dut.clk)            # E3: seq states set (top + child)
    await Timer(1, unit="ns")
    assert dut.acc_out.value == 0, f"acc_out latched too early = {dut.acc_out.value!s}"

    await RisingEdge(dut.clk)            # E4: child latches acc <= dbl
    await Timer(1, unit="ns")
    assert dut.acc_out.value == 42, f"acc_out = {dut.acc_out.value!s} (expected 42)"

    # Change src: comb path follows the very same cycle, latched acc must hold.
    dut.src_in.value = 5
    await Timer(1, unit="ns")
    assert dut.dbl_out.value == 10, f"dbl_out after src change = {dut.dbl_out.value!s} (expected 10)"
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert dut.acc_out.value == 42, f"acc_out moved after latch = {dut.acc_out.value!s} (expected 42)"


@cocotb.test()
async def check_hier_reset_hold(dut):
    # Under held master reset the child's acc stays at its reset value 0 while
    # the comb path keeps tracking src.
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value   = 1
    dut.src_in.value = 30
    for _ in range(4):
        await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert dut.acc_out.value == 0,  f"acc_out under mrst = {dut.acc_out.value!s} (expected 0)"
    assert dut.dbl_out.value == 60, f"dbl_out under mrst = {dut.dbl_out.value!s} (expected 60)"


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
