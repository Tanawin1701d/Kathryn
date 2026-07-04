# tc40 — deep hierarchy + sibling routing: Top { ChildA { GrandChild }, ChildB }.
#
# Covers:
#   * 2-level input chains: GrandChild reads Top's src (Top -> A -> G IoWires)
#   * 2-level output chains: Top reads GrandChild's dreg (G -> A -> Top IoWires)
#   * parent reads its own child: A reads G.deep
#   * sibling routing through the LCA: B reads A.stage (A -> Top -> B)
#   * IoWire reuse: A.stage is consumed by BOTH B and Top's out_stage, so the
#     A -> Top output chain must be built once and reused (find_reusable_io_wire)
#   * clk / mrst forwarding two levels down (G's clocked dreg)
#
# Comb dataflow (all continuous via wire defaults):
#   G.deep  = src + 1
#   A.stage = G.deep + 1      = src + 2
#   B.result= A.stage + 2     = src + 4
#   Top.out_res   = B.result  = src + 4
#   Top.out_stage = A.stage   = src + 2
# Clocked: G.dreg latches src once at the post-reset start pulse.

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc40_hier_deep_sibling"


# ---- model -------------------------------------------------------------------
class GrandChild(Module):
    # `self.top_src` is injected by Top (a Top-scope SignalRef, two levels up).
    @init
    def com_declare(self):
        self.deep = wire(8, "deep")
        self.dreg = reg (8, "dreg")

    @flow
    def my_flow(self):
        self.deep.default(self.top_src + 1)     # comb read of a signal 2 levels up
        self.dreg.reset(0)
        with seq():
            self.dreg |= self.top_src           # clocked 2 levels down


class ChildA(Module):
    @init
    def com_declare(self):
        self.gc    = GrandChild("gc")
        self.stage = wire(8, "stage")

    @flow
    def my_flow(self):
        self.stage.default(self.gc.deep + 1)    # parent reads its own child


class ChildB(Module):
    # `self.a_stage` (sibling A's wire) is injected by Top.
    @init
    def com_declare(self):
        self.result = wire(8, "result")

    @flow
    def my_flow(self):
        self.result.default(self.a_stage + 2)   # sibling read, routed via Top


class tc40_hier_deep_sibling(Module):
    @init
    def com_declare(self):
        self.src = wire(8, "src")
        self.src.mark_input("src_in")

        self.a = ChildA("child_a")
        self.b = ChildB("child_b")
        self.a.gc.top_src = self.src            # Top signal handed 2 levels down
        self.b.a_stage    = self.a.stage        # sibling handoff via Top

        self.out_res   = wire(8, "out_res")
        self.out_stage = wire(8, "out_stage")
        self.out_deep  = wire(8, "out_deep")
        self.out_res  .mark_output("res_out")
        self.out_stage.mark_output("stage_out")
        self.out_deep .mark_output("deep_out")

    @flow
    def my_flow(self):
        self.out_res  .default(self.b.result)   # Top reads B (1 hop)
        self.out_stage.default(self.a.stage)    # 2nd consumer of A.stage (reuse)
        self.out_deep .default(self.a.gc.dreg)  # Top reads GrandChild (2 hops)


# ---- build -------------------------------------------------------------------
def build(output_folder: str) -> None:
    reset()
    module = tc40_hier_deep_sibling()
    build_model(module)
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
@cocotb.test()
async def check_deep_and_sibling(dut):
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value   = 1
    dut.src_in.value = 7
    await RisingEdge(dut.clk)            # E1
    await RisingEdge(dut.clk)            # E2
    await Timer(1, unit="ns")
    dut.mrst.value = 0

    # Full comb chain settles regardless of the start sequence.
    assert dut.stage_out.value == 9,  f"stage_out = {dut.stage_out.value!s} (expected 9)"
    assert dut.res_out.value   == 11, f"res_out = {dut.res_out.value!s} (expected 11)"
    assert dut.deep_out.value  == 0,  f"deep_out before latch = {dut.deep_out.value!s} (expected 0)"

    await RisingEdge(dut.clk)            # E3: seq states set down to GrandChild
    await RisingEdge(dut.clk)            # E4: G latches dreg <= src
    await Timer(1, unit="ns")
    assert dut.deep_out.value == 7, f"deep_out = {dut.deep_out.value!s} (expected 7)"

    # Move src: every comb output tracks the same cycle, the latched dreg holds.
    dut.src_in.value = 100
    await Timer(1, unit="ns")
    assert dut.stage_out.value == 102, f"stage_out = {dut.stage_out.value!s} (expected 102)"
    assert dut.res_out.value   == 104, f"res_out = {dut.res_out.value!s} (expected 104)"
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert dut.deep_out.value == 7, f"deep_out moved after latch = {dut.deep_out.value!s} (expected 7)"


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
