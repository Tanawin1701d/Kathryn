# tc21 — pip/zync pipeline with a stage-2 conditional one-shot stall: a guard
# `cif((a + v) == v2)` fires `sywait(5)` exactly once, stalling the pipe for 5 cycles.
#
# Each stage is a `pip` (granter half) wrapping a `zync` (requester half). Adjacent
# stages SHARE an arbiter, so a stage's zync hands off to the next stage's pip:
#   * stage 1: pip(arb0, auto_req) → zync(arb1):  a <= a + 1   (counter)
#   * stage 2: pip(arb1) → seq[ cif((a+v)==v2): sywait(5) ; zync(arb2): b <= a ]
#   * stage 3: pip(arb2)           → zync(arb3, auto_ack):  c <= b   (follows b)
# arb0 (auto_req) is the always-requesting source end; arb3 (auto_ack) is the
# always-granted sink end.
#
# What sets tc21 apart from tc20: stage 2 guards its hand-off with a one-shot
# `cif((a + v) == v2)` → `sywait(5)`. With v == 1 and v2 == 6 the guard is true
# exactly once — when a reaches 5 — so the whole pipeline stalls for 5 cycles
# (a freezes at 6, b at 5) and then free-runs again at +1/cycle.
#
# Intended behaviour (what this testbench asserts):
#   * a is a free-running counter once the pipeline is flowing (monotonic, > 0).
#   * b tracks a and c tracks b with pipeline latency, so a >= b >= c holds at all
#     times and data eventually reaches every stage (b, c become non-zero).
#   * the stage-2 guard fires once: a single contiguous 5-cycle stall with a
#     frozen at v2, bracketed by +1/cycle free-running on either side.
#   * under held master reset every stage stays at its reset value 0.

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog
from kathryn.sim_assist import KSim

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc21_pip_zync_cond_stall"

RUN_CYCLES = 40                         # cycles to run after reset is released

# Stage-2 guard constants — must mirror the model's `v` / `v2` / `sywait`.
V          = 1                          # val(8, 1) added before the compare
V2         = 6                          # val(8, 6) compared against (a + v)
HOLD_CYCLES = 6                         # samples a sits at v2: the entry cycle + sywait(5) holds
STALL_SCAN  = 16                        # cycles to sample — brackets the lone stall (ends ~cyc 11)


# ---- model -------------------------------------------------------------------
class tc21_pip_zync_cond_stall(Module):
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
            with seq():
                with cif( (self.a + self.v) == self.v2):
                    sywait(5)
                with zync(self.pip_cons[2]):
                    self.b |= self.a

        with pip(self.pip_cons[2]):
            with zync(self.pip_cons[3], auto_ack=True):
                self.c |= self.b



# ---- build (kathryn model -> verilog) ---------------------------------------
def build(output_folder: str) -> None:
    reset()
    module = tc21_pip_zync_cond_stall()
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
async def check_conditional_stall(dut):
    # tc21's signature behaviour: the stage-2 guard `cif((a + v) == v2)` wraps a
    # `sywait(5)`, so the instant a reaches v2 the whole pipeline freezes. a is
    # frozen at v2 for exactly HOLD_CYCLES samples (the entry cycle + the 5 sywait
    # holds); the cycle it releases the pipeline steps to (7, 6, 5) and the next
    # to (8, 7, 6). Walk (a, b, c) and pin down that exact shape.
    k = KSim(dut)
    await _reset_and_release(dut)

    trace = []
    for _ in range(STALL_SCAN):
        await RisingEdge(dut.clk)
        await Timer(1, unit="ns")
        trace.append(_abc(k))

    a_seq = [a for a, _, _ in trace]

    # a must reach v2 and sit there for exactly HOLD_CYCLES consecutive samples.
    assert V2 in a_seq, f"a never reached {V2}: {a_seq}"
    start = a_seq.index(V2)
    assert a_seq[start : start + HOLD_CYCLES] == [V2] * HOLD_CYCLES, \
        f"a did not hold at {V2} for {HOLD_CYCLES} cycles: {a_seq}"
    assert start + HOLD_CYCLES + 1 < len(trace), f"scan too short to see the stall release: {trace}"

    # The cycle the stall releases, then the one after, step the whole pipeline
    # forward by one each: (7, 6, 5) then (8, 7, 6) for v2 == 6.
    after  = trace[start + HOLD_CYCLES]
    after2 = trace[start + HOLD_CYCLES + 1]
    assert after  == (V2 + 1, V2,     V2 - 1), f"first post-stall cycle wrong: {after}: {trace}"
    assert after2 == (V2 + 2, V2 + 1, V2),     f"second post-stall cycle wrong: {after2}: {trace}"


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
