# tc25 — tc24's multi-assign pipeline, but the two `a |= ...` writes in the stage-1
# block are wrapped in `with priority(...)` at DIFFERENT priorities. tc24 left both
# writes at the default priority, so program order decided (the LAST write won,
# a += V2). Here the HIGHEST priority is put on the FIRST-declared write, so if
# priority overrides program order (as it must), the first write wins instead:
#
#   stage 1: pip(arb0, auto_req) → zync(arb1):
#                with priority(PRI_HIGH): a |= a + v    # declared FIRST, +1, WINS
#                with priority(PRI_LOW):  a |= a + v2   # declared LAST,  +2, loses
#   stage 2: pip(arb1)          → zync(arb2):  b <= a
#   stage 3: pip(arb2)          → zync(arb3, auto_ack):  c <= b
#
# So a steps by V (the high-priority write) per grant, NOT by V2 — proving priority,
# not declaration order, decides the winner (cf. tc24, where order won at +V2).

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc25_pip_zync_multi_assign_priority"

RUN_CYCLES = 20                         # cycles to run after reset is released

V  = 1                                  # high-priority write addend: a |= a + v
V2 = 2                                  # low-priority  write addend: a |= a + v2

# Two priorities above the user default; the higher one is placed on the FIRST write.
PRI_HIGH = DEFAULT_UE_PRI_USER + 3
PRI_LOW  = DEFAULT_UE_PRI_USER + 1

# Priority decides, so the high-priority (first) write wins: a advances by V per grant.
WIN_STEP  = V                           # the winning (high-priority) write's addend
LOSE_STEP = V2                          # the losing (low-priority, last-declared) addend


# ---- model -------------------------------------------------------------------
class tc25_pip_zync_multi_assign_priority(Module):
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

        # stage 1 — assign `a` TWICE at different priorities. The high-priority write
        # is declared FIRST, so priority (not program order) makes it the winner.
        with pip(self.pip_cons[0], auto_req=True):
            with zync(self.pip_cons[1]):
                with priority(PRI_HIGH):
                    self.a |= self.a + self.v       # +1, declared FIRST, highest → WINS
                with priority(PRI_LOW):
                    self.a |= self.a + self.v2      # +2, declared LAST,  lowest  → loses

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
    module = tc25_pip_zync_multi_assign_priority()
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
async def check_priority_overrides_order(dut):
    # Two `a |= ...` writes share the stage-1 block at different priorities, with the
    # HIGH-priority write declared FIRST. Sample a's per-cycle step: it must equal
    # WIN_STEP (the high-priority write's addend, V), NOT LOSE_STEP (V2, which plain
    # program order would have picked as in tc24). That proves priority overrides
    # declaration order.
    await _reset_and_release(dut)

    a_seq = []
    for _ in range(RUN_CYCLES):
        await RisingEdge(dut.clk)
        await Timer(1, unit="ns")
        a_seq.append(_abc(dut)[0])

    # a must be a clean multiple-of-step ramp; collect the non-stall deltas.
    steps = [y - x for x, y in zip(a_seq, a_seq[1:]) if y != x]
    assert steps, f"a never advanced: {a_seq}"
    assert all(s == WIN_STEP for s in steps), \
        f"a stepped by {set(steps)}, expected only {WIN_STEP} (high-priority write wins): {a_seq}"
    assert LOSE_STEP not in steps, \
        f"a stepped by {LOSE_STEP} — the last-declared (low-priority) write wrongly won: {a_seq}"


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

# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
