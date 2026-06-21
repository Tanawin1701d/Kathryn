# tc16 — a 3-stage pip/zync pipeline chained through four shared arbiters.
#
# Each stage is a `pip` (granter half) wrapping a `zync` (requester half). Adjacent
# stages SHARE an arbiter, so a stage's zync hands off to the next stage's pip:
#   * stage 1: pip(arb0, auto_req) → zync(arb1):  a <= a + 1   (counter)
#   * stage 2: pip(arb1)           → zync(arb2):  b <= a       (follows a)
#   * stage 3: pip(arb2)           → zync(arb3, auto_ack):  c <= b   (follows b)
# arb0 (auto_req) is the always-requesting source end; arb3 (auto_ack) is the
# always-granted sink end.
#
# Intended behaviour (what this testbench asserts):
#   * a is a free-running counter once the pipeline is flowing (monotonic, > 0).
#   * b tracks a and c tracks b with pipeline latency, so a >= b >= c holds at all
#     times and data eventually reaches every stage (b, c become non-zero).
#   * under held master reset every stage stays at its reset value 0.
#
# NOTE: as of writing the model does not yet flow (a/b/c stay 0) — a one-cycle
# bootstrap race keeps stages 2/3 from arming. These tests encode the INTENDED
# behaviour and currently fail red; they pass once the handshake chain is fixed.

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc16_pip_zync"

RUN_CYCLES = 40                         # cycles to run after reset is released


# ---- model -------------------------------------------------------------------
class tc16_pip_zync(Module):
    @init
    def com_declare(self):
        # Four arbiters: one per stage boundary. Adjacent stages share one, so each
        # stage's zync hands off to the next stage's pip (arb0 = source, arb3 = sink).
        self.pip_cons = [PipCon() for i in range(4)]


        self.a = reg(8, "a")            # stage-1 counter (a += 1 per grant)
        self.b = reg(8, "b")            # stage-2 follows a
        self.c = reg(8, "c")            # stage-3 follows b
        self.v = val(8, 1, "v")

        self.a.mark_output("my_a")
        self.b.mark_output("my_b")
        self.c.mark_output("my_c")

    @flow
    def my_flow(self):
        # Three chained stages: stage N's zync shares its arbiter with stage N+1's
        # pip, so the grant of one stage feeds the request of the next.

        self.a.reset(0)
        self.b.reset(0)
        self.c.reset(0)

        # stage 1
        with pip(self.pip_cons[0], auto_req=True):
            with zync(self.pip_cons[1]):
                self.a |= self.a + self.v

        # stage 2
        with pip(self.pip_cons[1]):
            with zync(self.pip_cons[2]):
                self.b |= self.a

        with pip(self.pip_cons[2]):
            with zync(self.pip_cons[3], auto_ack=True):
                self.c |= self.b



# ---- build (kathryn model -> verilog) ---------------------------------------
def build(output_folder: str) -> None:
    reset()
    module = tc16_pip_zync()
    build_model(module)
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
async def _reset_and_release(dut):
    # Drive the clock, hold master reset for two edges (regs load 0, start arms),
    # then release it. Returns once reset is deasserted just after an edge.
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())
    dut.mrst.value = 1
    await RisingEdge(dut.clk)            # E1
    await RisingEdge(dut.clk)            # E2
    await Timer(1, unit="ns")
    dut.mrst.value = 0


def _abc(dut):
    return int(dut.my_a.value), int(dut.my_b.value), int(dut.my_c.value)


@cocotb.test()
async def check_stage1_counts(dut):
    # Stage 1 latches a <= a + 1 on every grant, so a must climb above 0 and never
    # step backwards once the pipeline is flowing (8-bit, no wrap within the run).
    await _reset_and_release(dut)

    prev_a = 0
    for _ in range(RUN_CYCLES):
        await RisingEdge(dut.clk)
        await Timer(1, unit="ns")
        a, _, _ = _abc(dut)
        assert a >= prev_a, f"a went backwards: {prev_a} -> {a}"
        prev_a = a

    assert prev_a > 0, f"stage 1 never counted: a stayed {prev_a}"


@cocotb.test()
async def check_pipeline_propagates(dut):
    # b follows a and c follows b, each a stage behind, so a >= b >= c holds at every
    # sample, all three are monotonic, and data eventually reaches the last stage.
    await _reset_and_release(dut)

    prev = (0, 0, 0)
    for _ in range(RUN_CYCLES):
        await RisingEdge(dut.clk)
        await Timer(1, unit="ns")
        cur = _abc(dut)
        a, b, c = cur
        assert a >= b >= c, f"pipeline order broken: a={a} b={b} c={c}"
        assert all(n >= p for n, p in zip(cur, prev)), f"a stage went backwards: {prev} -> {cur}"
        prev = cur

    a, b, c = prev
    assert b > 0, f"data never reached stage 2: b stayed {b}"
    assert c > 0, f"data never reached stage 3: c stayed {c}"


@cocotb.test()
async def check_reset_clears(dut):
    # While master reset is held every stage is pinned to its reset value 0.
    await _reset_and_release(dut)
    dut.mrst.value = 1                   # re-assert and keep it asserted
    for _ in range(4):
        await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert _abc(dut) == (0, 0, 0), f"reset did not clear the pipeline: {_abc(dut)}"

@cocotb.test()
async def check_debug(dut):
    # While master reset is held every stage is pinned to its reset value 0.
    await _reset_and_release(dut)
    for _ in range(20):
        await RisingEdge(dut.clk)


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
