# tc22 — pip/zync pipeline with a one-cycle stall bubble: a parallel
# `seq[ sywait(5); arb1.stall() ]` pulses arb1's hold once, freezing the pipe for 1 cycle.
#
# Each stage is a `pip` (granter half) wrapping a `zync` (requester half). Adjacent
# stages SHARE an arbiter, so a stage's zync hands off to the next stage's pip:
#   * stage 1: pip(arb0, auto_req) → zync(arb1):  a <= a + 1   (counter)
#   * stage 2: pip(arb1)           → zync(arb2):  b <= a       (follows a)
#   * stage 3: pip(arb2)           → zync(arb3, auto_ack):  c <= b   (follows b)
# arb0 (auto_req) is the always-requesting source end; arb3 (auto_ack) is the
# always-granted sink end.
#
# What sets tc22 apart from tc20: a parallel `seq[ sywait(5); arb1.stall() ]`
# pulses the stage-1/stage-2 arbiter's hold for a single cycle, 5 cycles in. That
# punches exactly one 1-cycle bubble into the otherwise free-running pipeline —
# a (and b) freeze for one cycle at a == 5, then free-run again at +1/cycle.
#
# Intended behaviour (what this testbench asserts):
#   * a is a free-running counter once the pipeline is flowing (monotonic, > 0).
#   * b tracks a and c tracks b with pipeline latency, so a >= b >= c holds at all
#     times and data eventually reaches every stage (b, c become non-zero).
#   * the stall fires once: a single 1-cycle bubble freezing a at 5, then the pipe
#     releases through (6,5,4) and (7,6,5) and free-runs after.
#   * under held master reset every stage stays at its reset value 0.

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog
from kathryn.sim_assist import KSim

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc22_pip_zync_stall_bubble"

RUN_CYCLES = 40                         # cycles to run after reset is released

# Stall constants — mirror the model's parallel `seq[ sywait(5); arb1.stall() ]`.
# a counts one per cycle, so the sywait(5) pulse lands with a frozen at 5.
STALL_AT   = 5                          # a's value when the lone 1-cycle bubble freezes stage 1
STALL_SCAN = 16                         # cycles to sample — brackets the lone bubble (~cyc 6)


# ---- model -------------------------------------------------------------------
class tc22_pip_zync_stall_bubble(Module):
    @init
    def com_declare(self):
        # Four arbiters: one per stage boundary. Adjacent stages share one, so each
        # stage's zync hands off to the next stage's pip (arb0 = source, arb3 = sink).
        self.pip_cons = [PipCon() for i in range(4)]


        self.a = reg(8, "a")            # stage-1 counter (a += 1 per grant)
        self.b = reg(8, "b")            # stage-2 follows a
        self.c = reg(8, "c")            # stage-3 follows b
        self.v = val(8, 1, "v")
        self.v2 = val(8, 6, "v2")

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


        with seq():
            sywait(5)
            self.pip_cons[1].stall()




# ---- build (kathryn model -> verilog) ---------------------------------------
def build(output_folder: str) -> None:
    reset()
    module = tc22_pip_zync_stall_bubble()
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


def _abc(k):
    return int(k.a.value), int(k.b.value), int(k.c.value)


@cocotb.test()
async def check_stage1_counts(dut):
    # Stage 1 latches a <= a + 1 on every grant, so a must climb above 0 and never
    # step backwards once the pipeline is flowing (8-bit, no wrap within the run).
    k = KSim(dut)
    await _reset_and_release(dut)

    prev_a = 0
    for _ in range(RUN_CYCLES):
        await RisingEdge(dut.clk)
        await Timer(1, unit="ns")
        a, _, _ = _abc(k)
        assert a >= prev_a, f"a went backwards: {prev_a} -> {a}"
        prev_a = a

    assert prev_a > 0, f"stage 1 never counted: a stayed {prev_a}"


@cocotb.test()
async def check_pipeline_propagates(dut):
    # b follows a and c follows b, each a stage behind, so a >= b >= c holds at every
    # sample, all three are monotonic, and data eventually reaches the last stage.
    k = KSim(dut)
    await _reset_and_release(dut)

    prev = (0, 0, 0)
    for _ in range(RUN_CYCLES):
        await RisingEdge(dut.clk)
        await Timer(1, unit="ns")
        cur = _abc(k)
        a, b, c = cur
        assert a >= b >= c, f"pipeline order broken: a={a} b={b} c={c}"
        assert all(n >= p for n, p in zip(cur, prev)), f"a stage went backwards: {prev} -> {cur}"
        prev = cur

    a, b, c = prev
    assert b > 0, f"data never reached stage 2: b stayed {b}"
    assert c > 0, f"data never reached stage 3: c stayed {c}"


@cocotb.test()
async def check_one_shot_stall(dut):
    # tc22's signature behaviour: a parallel `seq[ sywait(5); arb1.stall() ]` pulses
    # the stage-1/stage-2 arbiter's hold for a single cycle, 5 cycles in. That
    # punches exactly one 1-cycle bubble into the otherwise free-running counter:
    # a freezes for one cycle at STALL_AT, then resumes +1/cycle forever.
    k = KSim(dut)
    await _reset_and_release(dut)

    trace = []
    for _ in range(STALL_SCAN):
        await RisingEdge(dut.clk)
        await Timer(1, unit="ns")
        trace.append(_abc(k))

    a_seq = [a for a, _, _ in trace]

    # Every cycle-to-cycle step is either a free-run advance (+1) or the stall hold
    # (0) — a never jumps or rewinds.
    deltas = [cur - prev for prev, cur in zip(a_seq, a_seq[1:])]
    assert all(d in (0, 1) for d in deltas), f"a stepped illegally: {a_seq}"

    # Exactly one hold, and a is frozen at STALL_AT when it happens.
    held = [i for i, d in enumerate(deltas) if d == 0]
    assert len(held) == 1, f"expected exactly one 1-cycle stall, holds at {held}: {a_seq}"
    assert a_seq[held[0]] == STALL_AT, f"a stalled at {a_seq[held[0]]}, expected {STALL_AT}: {a_seq}"

    # The bubble's three cycles, explicitly: it freezes at (5, 4, 4), then the pipe
    # releases to (6, 5, 4) and (7, 6, 5).
    frozen = held[0] + 1                          # index of the held sample (a's second STALL_AT)
    assert frozen + 2 < len(trace), f"scan too short to see the stall release: {trace}"
    assert trace[frozen]     == (STALL_AT,     STALL_AT - 1, STALL_AT - 1), f"stall cycle wrong: {trace[frozen]}: {trace}"
    assert trace[frozen + 1] == (STALL_AT + 1, STALL_AT,     STALL_AT - 1), f"release cycle wrong: {trace[frozen + 1]}: {trace}"
    assert trace[frozen + 2] == (STALL_AT + 2, STALL_AT + 1, STALL_AT),     f"post-release cycle wrong: {trace[frozen + 2]}: {trace}"


@cocotb.test()
async def check_reset_clears(dut):
    # While master reset is held every stage is pinned to its reset value 0.
    k = KSim(dut)
    await _reset_and_release(dut)
    dut.mrst.value = 1                   # re-assert and keep it asserted
    for _ in range(4):
        await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert _abc(k) == (0, 0, 0), f"reset did not clear the pipeline: {_abc(k)}"

# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
