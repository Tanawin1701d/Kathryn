# tc27 — conditional fan-out: one producer `a` routes to a DIFFERENT consumer
# pipeline depending on its parity, via a per-bind-conditioned multi-arb zync.
#
# Like tc26 the producer's zync contends on two downstream arbiters, but here each
# bind carries a `cond` that gates both its REQ (`state_exit & cond`) and its grant
# term (`ack & cond`). With mode="any" (OR) exactly one bind fires per grant — the
# one whose parity condition is currently true:
#   * stage 1   : pip(arb0, auto_req) → zync([(arb_e, ~(a+v)[0]), (arb_o, (a+v)[0])]):  a <= a + 1
#   * even path : pip(arb_e)          → zync(arb_se, auto_ack):                         be <= a
#   * odd path  : pip(arb_o)          → zync(arb_so, auto_ack):                         bo <= a
# CAREFUL: the consumers read `a` AFTER stage 1 increments it, so they latch the NEXT
# value. The cond must therefore be the parity of `a + v` (what gets delivered), not of
# `a` — otherwise even/odd routing is inverted. `~(a+v)[0]` (next a even) gates the even
# bind; `(a+v)[0]` (next a odd) gates the odd bind. a advances every grant (one parity
# is always true), so it free-runs.
#
# Intended behaviour (what this testbench asserts):
#   * a is a free-running counter once the pipeline is flowing (monotonic, > 0).
#   * both paths are exercised: be and bo each eventually become non-zero, and each
#     lags the producer (a >= be, a >= bo) and never steps backwards.
#   * routing is by parity: the even path only ever captures EVEN values of a (be is
#     always even) and the odd path only ODD values (bo is 0 or odd).
#   * under held master reset every register stays at its reset value 0.
#
# NOTE: like tc20/tc26, the model may not yet flow (a/be/bo stay 0) pending the
# handshake bootstrap fix. These tests encode the INTENDED behaviour.

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog
from kathryn.sim_assist import KSim

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc27_zync_parity_fanout"

RUN_CYCLES = 40                         # cycles to run after reset is released


# ---- model -------------------------------------------------------------------
class tc27_zync_parity_fanout(Module):
    @init
    def com_declare(self):
        # Five arbiters: arb0 = source, arb_e/arb_o = the even/odd fan-out
        # boundaries (a's zync conditions on each), arb_se/arb_so = the two sinks.
        self.pip_cons = [PipCon() for i in range(5)]

        self.a  = reg(8, "a")           # producer counter (a += 1 per grant)
        self.be = reg(8, "be")          # even path: captures a when the delivered a is even
        self.bo = reg(8, "bo")          # odd  path: captures a when the delivered a is odd
        self.v  = val(8, 1, "v")

    @flow
    def my_flow(self):
        # Parity-routed fan-out: a's zync binds the even arbiter under `~(a+v)[0]`
        # (delivered value even) and the odd arbiter under `(a+v)[0]` (delivered value
        # odd); mode="any" lets the single satisfied bind fire each grant.

        self.a.reset(0)
        self.be.reset(0)
        self.bo.reset(0)

        # stage 1 — route the grant by the parity of the value the consumers will
        # latch. The consumers read `a` AFTER this block has done `a |= a + v`, so
        # they capture the incremented value — route on `(a + v)`, not on `a`.
        with pip(self.pip_cons[0], auto_req=True):
            with zync(
                [(self.pip_cons[1], ~(self.a + self.v)[0]),    # even bind: next a is even
                 (self.pip_cons[2],  (self.a + self.v)[0])],   # odd  bind: next a is odd
                mode="any",
            ):
                self.a |= self.a + self.v

        # even path — only granted when the delivered a is even
        with pip(self.pip_cons[1]):
            with zync(self.pip_cons[3], auto_ack=True):
                self.be |= self.a

        # odd path — only granted when the delivered a is odd
        with pip(self.pip_cons[2]):
            with zync(self.pip_cons[4], auto_ack=True):
                self.bo |= self.a



# ---- build (kathryn model -> verilog) ---------------------------------------
def build(output_folder: str) -> None:
    reset()
    module = tc27_zync_parity_fanout()
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


def _vals(k):
    return int(k.a.value), int(k.be.value), int(k.bo.value)


@cocotb.test()
async def check_producer_counts(dut):
    # a advances on every grant (one parity is always true), so it must climb above
    # 0 and never step backwards once the pipeline is flowing (8-bit, no wrap in run).
    k = KSim(dut)
    await _reset_and_release(dut)

    prev_a = 0
    for _ in range(RUN_CYCLES):
        await RisingEdge(dut.clk)
        await Timer(1, unit="ns")
        a, _, _ = _vals(k)
        assert a >= prev_a, f"a went backwards: {prev_a} -> {a}"
        prev_a = a

    assert prev_a > 0, f"producer never counted: a stayed {prev_a}"


@cocotb.test()
async def check_both_paths_fire(dut):
    # Both consumers lag the producer (a >= be, a >= bo), each is monotonic, and
    # since a alternates parity both paths eventually capture data.
    k = KSim(dut)
    await _reset_and_release(dut)

    prev = (0, 0, 0)
    for _ in range(RUN_CYCLES):
        await RisingEdge(dut.clk)
        await Timer(1, unit="ns")
        cur = _vals(k)
        a, be, bo = cur
        assert a >= be, f"even path outran producer: a={a} be={be}"
        assert a >= bo, f"odd path outran producer: a={a} bo={bo}"
        assert all(n >= p for n, p in zip(cur, prev)), f"a register went backwards: {prev} -> {cur}"
        prev = cur

    a, be, bo = prev
    assert be > 0, f"even path never fired: be stayed {be}"
    assert bo > 0, f"odd path never fired: bo stayed {bo}"


@cocotb.test()
async def check_parity_routing(dut):
    # The cond on each bind routes by parity: the even path only captures even a
    # (be is always even), the odd path only odd a (bo is 0 at reset, else odd).
    k = KSim(dut)
    await _reset_and_release(dut)

    for _ in range(RUN_CYCLES):
        await RisingEdge(dut.clk)
        await Timer(1, unit="ns")
        _, be, bo = _vals(k)
        assert be % 2 == 0, f"even path captured an odd value: be={be}"
        assert bo == 0 or bo % 2 == 1, f"odd path captured an even value: bo={bo}"


@cocotb.test()
async def check_reset_clears(dut):
    # While master reset is held every register is pinned to its reset value 0.
    k = KSim(dut)
    await _reset_and_release(dut)
    dut.mrst.value = 1                   # re-assert and keep it asserted
    for _ in range(4):
        await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert _vals(k) == (0, 0, 0), f"reset did not clear the registers: {_vals(k)}"

# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
