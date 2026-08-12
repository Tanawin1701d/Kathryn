# tc26 — multi-pipeline fan-out: one producer `a` fires TWO consumer pipelines,
# b0 and b1, in lockstep via a multi-arb zync (mode="all").
#
# Stage 1's zync contends on BOTH downstream arbiters at once, so a single grant
# of `a` hands off to both consumers. Each consumer is its own one-stage pipeline:
#   * stage 1 : pip(arb0, auto_req) → zync([arb1a, arb1b], mode="all"):  a <= a + 1
#   * stage b0: pip(arb1a)          → zync(arb_s0, auto_ack):            b0 <= a
#   * stage b1: pip(arb1b)          → zync(arb_s1, auto_ack):            b1 <= a
# arb0 (auto_req) is the always-requesting source; arb_s0/arb_s1 (auto_ack) are the
# always-granted sinks. mode="all" means a only advances once BOTH b0 and b1 ack,
# so the two consumer pipelines stay in lockstep.
#
# Intended behaviour (what this testbench asserts):
#   * a is a free-running counter once the pipeline is flowing (monotonic, > 0).
#   * b0 and b1 each track a a stage behind, so a >= b0 and a >= b1 at all times
#     and both consumers eventually become non-zero.
#   * the fan-out is symmetric: b0 == b1 at every sample (same producer, same
#     latency, locked by mode="all").
#   * under held master reset every register stays at its reset value 0.
#
# NOTE: like tc20, the model may not yet flow (a/b0/b1 stay 0) pending the handshake
# bootstrap fix. These tests encode the INTENDED behaviour and pass once it lands.

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc26_zync_fanout"

RUN_CYCLES = 40                         # cycles to run after reset is released


# ---- model -------------------------------------------------------------------
class tc26_zync_fanout(Module):
    @init
    def com_declare(self):
        # Five arbiters: arb0 = source, arb1a/arb1b = the two fan-out boundaries
        # (a's zync drives both), arb_s0/arb_s1 = the two consumer sinks.
        self.pip_cons = [PipCon() for i in range(5)]

        self.a  = reg(8, "a")           # producer counter (a += 1 per grant)
        self.b0 = reg(8, "b0")          # consumer 0 follows a
        self.b1 = reg(8, "b1")          # consumer 1 follows a
        self.v  = val(8, 1, "v")

        self.a.mark_output("my_a")
        self.b0.mark_output("my_b0")
        self.b1.mark_output("my_b1")

    @flow
    def my_flow(self):
        # One producer fans out to two consumers: a's zync contends on both arb1a
        # and arb1b (mode="all"), so its grant fires b0 and b1 together.

        self.a.reset(0)
        self.b0.reset(0)
        self.b1.reset(0)

        # stage 1 — producer fires both downstream arbiters at once
        with pip(self.pip_cons[0], auto_req=True):
            with zync([self.pip_cons[1], self.pip_cons[2]], mode="all"):
                self.a |= self.a + self.v

        # consumer 0
        with pip(self.pip_cons[1]):
            with zync(self.pip_cons[3], auto_ack=True):
                self.b0 |= self.a

        # consumer 1
        with pip(self.pip_cons[2]):
            with zync(self.pip_cons[4], auto_ack=True):
                self.b1 |= self.a



# ---- build (kathryn model -> verilog) ---------------------------------------
def build(output_folder: str) -> None:
    reset()
    module = tc26_zync_fanout()
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


def _vals(dut):
    return int(dut.my_a.value), int(dut.my_b0.value), int(dut.my_b1.value)


@cocotb.test()
async def check_producer_counts(dut):
    # Stage 1 latches a <= a + 1 on every (joint) grant, so a must climb above 0
    # and never step backwards once the pipeline is flowing (8-bit, no wrap in run).
    await _reset_and_release(dut)

    prev_a = 0
    for _ in range(RUN_CYCLES):
        await RisingEdge(dut.clk)
        await Timer(1, unit="ns")
        a, _, _ = _vals(dut)
        assert a >= prev_a, f"a went backwards: {prev_a} -> {a}"
        prev_a = a

    assert prev_a > 0, f"producer never counted: a stayed {prev_a}"


@cocotb.test()
async def check_fanout_propagates(dut):
    # b0 and b1 each follow a a stage behind, so a >= b0 and a >= b1 at every
    # sample, all three are monotonic, and both consumers eventually fire.
    await _reset_and_release(dut)

    prev = (0, 0, 0)
    for _ in range(RUN_CYCLES):
        await RisingEdge(dut.clk)
        await Timer(1, unit="ns")
        cur = _vals(dut)
        a, b0, b1 = cur
        assert a >= b0, f"consumer 0 outran producer: a={a} b0={b0}"
        assert a >= b1, f"consumer 1 outran producer: a={a} b1={b1}"
        assert all(n >= p for n, p in zip(cur, prev)), f"a register went backwards: {prev} -> {cur}"
        prev = cur

    a, b0, b1 = prev
    assert b0 > 0, f"data never reached consumer 0: b0 stayed {b0}"
    assert b1 > 0, f"data never reached consumer 1: b1 stayed {b1}"


@cocotb.test()
async def check_fanout_symmetric(dut):
    # mode="all" locks both consumers to the same producer grant, so the two
    # pipelines stay identical: b0 == b1 at every sample.
    await _reset_and_release(dut)

    for _ in range(RUN_CYCLES):
        await RisingEdge(dut.clk)
        await Timer(1, unit="ns")
        _, b0, b1 = _vals(dut)
        assert b0 == b1, f"fan-out diverged: b0={b0} b1={b1}"


@cocotb.test()
async def check_reset_clears(dut):
    # While master reset is held every register is pinned to its reset value 0.
    await _reset_and_release(dut)
    dut.mrst.value = 1                   # re-assert and keep it asserted
    for _ in range(4):
        await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert _vals(dut) == (0, 0, 0), f"reset did not clear the registers: {_vals(dut)}"

# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
