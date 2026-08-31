# tc24 — same pip/zync pipeline shape as tc20, but stage 1 assigns `a` TWICE in
# the same clocked block to probe override semantics: does the second `a |= ...`
# win (last-write override), the first win, or do both somehow apply?
#
#   stage 1: pip(arb0, auto_req) → zync(arb1):
#                a |= a + v     # first  write: a + 1
#                a |= a + v2    # second write: a + 2
#   stage 2: pip(arb1)          → zync(arb2):  b <= a       (follows a)
#   stage 3: pip(arb2)          → zync(arb3, auto_ack):  c <= b   (follows b)
#
# The per-grant step of `a` reveals the winner:
#   * +2 per cycle  → the LAST write overrides (a + v2),  a = 0, 2, 4, ...
#   * +1 per cycle  → the FIRST write wins   (a + v),     a = 0, 1, 2, ...
#   * +3 per cycle  → both writes accumulate (a + v + v2)
# This testbench pins down whichever the model actually does (see OVERRIDE_STEP).

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog
from kathryn.sim_assist import KSim

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc24_pip_zync_multi_assign_order"

RUN_CYCLES = 20                         # cycles to run after reset is released

V  = 1                                  # first  write addend: a |= a + v
V2 = 2                                  # second write addend: a |= a + v2

# Observed semantics: the second `a |= ...` in the block overrides the first, so a
# advances by V2 each grant (last-write-wins). Asserted in check_last_write_wins.
OVERRIDE_STEP = V2


# ---- model -------------------------------------------------------------------
class tc24_pip_zync_multi_assign_order(Module):
    @init
    def com_declare(self):
        # Four arbiters: one per stage boundary. Adjacent stages share one, so each
        # stage's zync hands off to the next stage's pip (arb0 = source, arb3 = sink).
        self.pip_cons = [PipCon() for i in range(4)]

        self.a = reg(8, "a")            # stage-1 counter, assigned twice per grant
        self.b = reg(8, "b")            # stage-2 follows a
        self.c = reg(8, "c")            # stage-3 follows b
        self.v  = val(8, V,  "v")
        self.v2 = val(8, V2, "v2")

    @flow
    def my_flow(self):
        # Three chained stages: stage N's zync shares its arbiter with stage N+1's
        # pip, so the grant of one stage feeds the request of the next.

        self.a.reset(0)
        self.b.reset(0)
        self.c.reset(0)

        # stage 1 — assign `a` TWICE in the same clocked block; the second should
        # override the first if the model is last-write-wins.
        with pip(self.pip_cons[0], auto_req=True):
            with zync(self.pip_cons[1]):
                self.a |= self.a + self.v       # first  write: a + 1
                self.a |= self.a + self.v2      # second write: a + 2 (expected winner)

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
    module = tc24_pip_zync_multi_assign_order()
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
async def check_last_write_wins(dut):
    # Two `a |= ...` writes share the stage-1 block. Sample a's per-cycle step once
    # the pipeline is flowing: it must equal OVERRIDE_STEP (the SECOND write's
    # addend), proving the later assignment overrode the earlier one rather than the
    # first winning (+V) or both accumulating (+V+V2).
    k = KSim(dut)
    await _reset_and_release(dut)

    a_seq = []
    for _ in range(RUN_CYCLES):
        await RisingEdge(dut.clk)
        await Timer(1, unit="ns")
        a_seq.append(_abc(k)[0])

    # a must be a clean multiple-of-step ramp; collect the non-stall deltas.
    steps = [y - x for x, y in zip(a_seq, a_seq[1:]) if y != x]
    assert steps, f"a never advanced: {a_seq}"
    assert all(s == OVERRIDE_STEP for s in steps), \
        f"a stepped by {set(steps)}, expected only {OVERRIDE_STEP} (last write wins): {a_seq}"
    assert a_seq[-1] >= OVERRIDE_STEP, f"a did not count: {a_seq}"


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
