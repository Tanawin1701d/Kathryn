# tc40 — PipCon.no_pip_master(): a zync sink arbiter that NO pip block masters.
#
# Stage 1 is the usual producer (pip auto_req → zync), but its zync contends on a
# PipCon whose master-ack is hard-tied to constant 1 via no_pip_master() instead
# of being driven by a downstream consumer pip:
#   * stage 1: pip(arb_src, auto_req) → zync(arb_sink):  a <= a + 1
#   * arb_sink: no_pip_master()  — leaf ack = req & 1, granted the moment it asks.
#
# Intended behaviour (what this testbench asserts):
#   * the sink always grants, so the producer NEVER stalls: once a starts moving
#     it increments by exactly 1 every cycle (any plateau = a lost grant).
#   * a is monotonic and becomes non-zero (the pipeline actually flows).
#   * under held master reset a stays at its reset value 0.

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc40_no_pip_master"

RUN_CYCLES = 40                         # cycles to run after reset is released


# ---- model -------------------------------------------------------------------
class tc40_no_pip_master(Module):
    @init
    def com_declare(self):
        self.arb_src  = PipCon()        # producer gate (always-requesting pip)
        self.arb_sink = PipCon()        # sink boundary — no consumer pip masters it
        self.arb_sink.no_pip_master()   # master-ack hard-tied to 1

        self.a = reg(8, "a")            # producer counter (a += 1 per grant)
        self.v = val(8, 1, "v")

        self.a.mark_output("my_a")

    @flow
    def my_flow(self):
        self.a.reset(0)

        # producer: requests every cycle; the no-master sink grants every request.
        with pip(self.arb_src, auto_req=True):
            with zync(self.arb_sink):
                self.a |= self.a + self.v


# ---- build (kathryn model -> verilog) ---------------------------------------
def build(output_folder: str) -> None:
    reset()
    module = tc40_no_pip_master()
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


@cocotb.test()
async def check_never_stalls(dut):
    # The no-master sink acks every request, so once the counter starts moving it
    # gains exactly 1 per cycle — a repeated value means a grant was lost.
    await _reset_and_release(dut)

    prev_a  = 0
    started = False
    for _ in range(RUN_CYCLES):
        await RisingEdge(dut.clk)
        await Timer(1, unit="ns")
        a = int(dut.my_a.value)
        if started:
            assert a == prev_a + 1, f"producer stalled: {prev_a} -> {a}"
        elif a != 0:
            started = True
            assert a == 1, f"counter did not start from 1: {a}"
        prev_a = a

    assert started, "producer never counted: a stayed 0"


@cocotb.test()
async def check_reset_clears(dut):
    # While master reset is held the counter is pinned to its reset value 0.
    await _reset_and_release(dut)
    dut.mrst.value = 1                   # re-assert and keep it asserted
    for _ in range(4):
        await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert int(dut.my_a.value) == 0, f"reset did not clear the counter: {int(dut.my_a.value)}"


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
