# tc42 — pip auto_restart: a ONE-cycle flush on an always-requesting pip clears
# it and re-enters it in the same cycle, so only the flush cycle itself is lost.
# The same flush on a pip WITHOUT the flag leaves it dead — the control, and the
# plain semantics.
#
# The mechanism is the StateReg ladder, not the pip wiring: DEFAULT_UE_PRI_SR_SET
# sits ABOVE DEFAULT_UE_PRI_SR_RST, so a state cleared and entered in one cycle
# comes out entered. `auto_restart` only supplies the entrance edge that asserts
# that SET while the flush is up.
#
#   tick  pip(auto_req) -> zync(auto_ack): tick += 1; zif(tick == FLUSH_AT): flush both
#   live  pip(auto_req, auto_restart) -> zync(auto_ack): live += 1   flushed once
#   dead  pip(auto_req)               -> zync(auto_ack): dead += 1   flushed once
#
# The tick zync is active every cycle, so the zif inside it shapes the flush
# into a ONE-cycle pulse — the shape a branch complex flushes a core with.
# (tc23's `seq[sywait(5); flush()]` is the held-level form.)
#
# What this testbench asserts:
#   * the flush wire is high in exactly one sample
#   * in that sample the live pip IS still granted — it is re-entered as it is
#     cleared, which is the whole point
#   * live loses exactly ONE cycle (the flush is seen at sample `at` and applied
#     on the edge leaving it, so the gap shows at at+1), then counts to the end
#   * dead never changes after the pulse
#   * under held master reset every counter is 0

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog
from kathryn.sim.ksim import KSim

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc42_pip_auto_restart"

FLUSH_AT   = 6                          # tick value the flush fires on
RUN_CYCLES = 30                         # samples taken after reset is released


# ---- model -------------------------------------------------------------------
class tc42_pip_auto_restart(Module):
    @init
    def com_declare(self):
        self.tick_con, self.tick_sink = PipCon(), PipCon()
        self.live_con, self.live_sink = PipCon(), PipCon()
        self.dead_con, self.dead_sink = PipCon(), PipCon()
        self.tick  = reg(8, "tick")     # free-running cycle count
        self.live  = reg(8, "live")     # counts through the restart pip
        self.dead  = reg(8, "dead")     # counts through the plain pip: the control
        self.pulse = wire(1, "pulse")   # the flush wire, read through KSim
        self.grant = wire(1, "grant")   # the live pip's own leaf ack

    @flow
    def my_flow(self):
        for counter in (self.tick, self.live, self.dead):
            counter.reset(0)

        with pip(self.tick_con, auto_req=True):
            with zync(self.tick_sink, auto_ack=True):
                self.tick |= self.tick + 1
                with zif(self.tick == FLUSH_AT):
                    self.pulse *= self.live_con.flush()
                    self.dead_con.flush()

        with pip(self.live_con, auto_req=True, auto_restart=True):
            with zync(self.live_sink, auto_ack=True):
                self.live |= self.live + 1

        with pip(self.dead_con, auto_req=True):
            with zync(self.dead_sink, auto_ack=True):
                self.dead |= self.dead + 1

        self.grant *= self.live_con.leaf(0).ack


# ---- build (kathryn model -> verilog) ---------------------------------------
def build(output_folder: str) -> None:
    reset()
    module = tc42_pip_auto_restart()
    build_model(module)
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
async def _reset_and_release(dut):
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())
    dut.mrst.value = 1
    await RisingEdge(dut.clk)
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    dut.mrst.value = 0


async def _sample_run(dut, cycles: int):
    # One tuple per cycle, read just after each rising edge.
    k = KSim(dut)
    await _reset_and_release(dut)
    samples = []
    for _ in range(cycles):
        await RisingEdge(dut.clk)
        await Timer(1, unit="ns")
        samples.append((int(k.tick.value), int(k.pulse.value), int(k.grant.value),
                        int(k.live.value), int(k.dead.value)))
    return samples


def _pulse_index(samples) -> int:
    pulses = [i for i, s in enumerate(samples) if s[1] == 1]
    assert len(pulses) == 1, f"the flush should pulse once, saw it at {pulses}"
    return pulses[0]


@cocotb.test()
async def check_restart_pip_loses_only_the_flush_cycle(dut):
    samples = await _sample_run(dut, RUN_CYCLES)
    at      = _pulse_index(samples)
    grants  = [s[2] for s in samples]
    counts  = [s[3] for s in samples]
    window  = samples[at - 1:at + 4]

    # The pip is re-entered as it is cleared, so its grant never drops.
    assert grants[at] == 1, f"the live pip lost its grant during the flush: {window}"

    # The flush is SEEN at sample `at` and APPLIED on the edge leaving it, so the
    # one cycle it costs shows up at at+1.
    assert counts[at + 1] == counts[at], f"the flush cleared nothing: {window}"

    # ...and exactly one: counting resumes immediately after, nothing spent parking.
    assert counts[at + 2] == counts[at + 1] + 1, f"live lost more than the flush cycle: {window}"

    for i in range(at + 2, RUN_CYCLES):
        assert counts[i] == counts[i - 1] + 1, f"live did not count at sample {i}: {counts}"


@cocotb.test()
async def check_plain_pip_stays_dead_after_the_flush(dut):
    samples = await _sample_run(dut, RUN_CYCLES)
    at      = _pulse_index(samples)
    deads   = [s[4] for s in samples]
    ticks   = [s[0] for s in samples]

    assert deads[at] > 0, f"the plain pip never counted before the flush: {deads}"
    assert all(d == deads[at] for d in deads[at:]), f"the plain pip moved after its flush: {deads}"
    assert ticks[-1] > FLUSH_AT + 10, f"the free-running count stopped: {ticks}"


@cocotb.test()
async def check_reset_clears(dut):
    k = KSim(dut)
    await _reset_and_release(dut)
    dut.mrst.value = 1
    for _ in range(4):
        await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    counters = (int(k.tick.value), int(k.live.value), int(k.dead.value))
    assert counters == (0, 0, 0), f"reset did not clear the counters: {counters}"


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
