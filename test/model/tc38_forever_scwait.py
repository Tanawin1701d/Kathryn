# tc38 — the "processor loop" pattern: an endless cwhile whose body handshakes
# with the outside world through scwait, plus always-on comb logic beside the
# loop (bare assigns outside any flow block).
#   with cwhile(halted == 0):      # spins until halted is set
#       scwait(go)                 # wait for external strobe
#       cnt |= cnt + 1             # do one unit of work
#       with zif(cnt == LIMIT-1): halted |= 1
#   mirror *= cnt + 100            # bare comb: always active, not state-gated

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME  = "tc38_forever_scwait"
LIMIT = 3


# ---- model -------------------------------------------------------------------
class tc38_forever_scwait(Module):
    @init
    def com_declare(self):
        self.cnt    = reg (8, "cnt")
        self.halted = reg (1, "halted")
        self.go     = wire(1, "go")
        self.mirror = wire(8, "mirror")

        self.go.mark_input     ("go_in")
        self.cnt.mark_output   ("cnt_out")
        self.halted.mark_output("halted_out")
        self.mirror.mark_output("mirror_out")

    @flow
    def my_flow(self):
        self.cnt.reset(0)
        self.halted.reset(0)

        # always-on comb beside the loop: bare assign, never state-gated
        self.mirror *= self.cnt + 100

        with seq():
            with cwhile(self.halted == 0):
                scwait(self.go)                       # handshake with the outside
                self.cnt |= self.cnt + 1
                with zif(self.cnt == LIMIT - 1):      # zero-cycle: same statement
                    self.halted |= 1


# ---- build -------------------------------------------------------------------
def build(output_folder: str) -> None:
    reset()
    build_model(tc38_forever_scwait())
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
async def _reset(dut):
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())
    dut.mrst.value  = 1
    dut.go_in.value = 0
    await RisingEdge(dut.clk)
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    dut.mrst.value = 0


async def _pulse_go(dut, cycles_high=1):
    dut.go_in.value = 1
    for _ in range(cycles_high):
        await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    dut.go_in.value = 0


@cocotb.test()
async def check_loop_handshake(dut):
    # Each go strobe advances cnt exactly once; after LIMIT strobes the loop
    # sets halted and stops reacting to further strobes.
    await _reset(dut)

    # no strobe → cnt must stay put
    for _ in range(6):
        await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert int(dut.cnt_out.value) == 0, f"cnt moved without go: {dut.cnt_out.value!s}"
    assert int(dut.halted_out.value) == 0

    # strobe LIMIT times → cnt counts each one
    for i in range(1, LIMIT + 1):
        await _pulse_go(dut)
        for _ in range(4):                     # let the loop swing back around
            await RisingEdge(dut.clk)
        await Timer(1, unit="ns")
        assert int(dut.cnt_out.value) == i, \
            f"after strobe {i}: cnt = {dut.cnt_out.value!s} (expected {i})"

    await Timer(1, unit="ns")
    assert int(dut.halted_out.value) == 1, "halted did not set after LIMIT strobes"

    # further strobes must be ignored once halted
    for _ in range(2):
        await _pulse_go(dut)
        for _ in range(3):
            await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert int(dut.cnt_out.value) == LIMIT, \
        f"cnt advanced after halt: {dut.cnt_out.value!s} (expected {LIMIT})"


@cocotb.test()
async def check_bare_comb_always_active(dut):
    # mirror = cnt + 100 must track cnt in EVERY state (waiting, working, halted)
    # — proves bare comb assigns are not gated by the loop's state machine.
    await _reset(dut)

    async def expect_mirror():
        await Timer(1, unit="ns")
        c, m = int(dut.cnt_out.value), int(dut.mirror_out.value)
        assert m == c + 100, f"mirror = {m} but cnt = {c}"

    await expect_mirror()                      # while parked in scwait
    for _ in range(LIMIT):
        await _pulse_go(dut)
        for _ in range(3):
            await RisingEdge(dut.clk)
        await expect_mirror()                  # after each work step
    await expect_mirror()                      # after halt


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
