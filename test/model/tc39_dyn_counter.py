# tc39 — dynamic counter (DynCounter CCP): a clocked register plus a chained
# conditional add committed once per cycle.
#   cnt  = counter(3)            # 3-bit, wraps mod 8
#   cnt.add(2, en1)              # stage 0: en1 ? cnt + 2 : cnt
#   cnt.add(3, en2)              # stage 1: en2 ? st0 + 3 : st0   (adds accumulate)
#   now_probe *= cnt.now         # comb chain head, BEFORE the commit
#   cnt.update()                 # cnt <= chain head (bare clocked statement)
# plus a second, enable-less counter free-running at +1 per cycle (4-bit).
# The bare update() exercises the module-level basic-node path: the module hands
# its clk straight to the node, so the commit lands in `always @(posedge clk)`.

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog
from kathryn.sim_assist import KSim

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc39_dyn_counter"


# ---- model -------------------------------------------------------------------
class tc39_dyn_counter(Module):
    @init
    def com_declare(self):
        self.en1       = wire(1, "en1")
        self.en2       = wire(1, "en2")
        self.cnt       = counter(3, "cnt")
        self.free      = counter(4, "free")
        self.now_probe = wire(3, "now_probe")

        self.en1.mark_input        ("en1_in")
        self.en2.mark_input        ("en2_in")

    @flow
    def my_flow(self):
        self.cnt.reset(0)
        self.free.reset(0)

        # conditional adds accumulate: cnt' = cnt + 2*en1 + 3*en2 (mod 8)
        self.cnt.add(2, self.en1)
        self.cnt.add(3, self.en2)
        self.now_probe *= self.cnt.now      # chain head, sampled before commit
        self.cnt.update()

        # enable-less chain: a free-running +1 counter (mod 16)
        self.free.add(1)
        self.free.update()


# ---- build -------------------------------------------------------------------
def build(output_folder: str) -> None:
    reset()
    build_model(tc39_dyn_counter())
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
async def _reset(dut):
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())
    dut.mrst.value   = 1
    dut.en1_in.value = 0
    dut.en2_in.value = 0
    await RisingEdge(dut.clk)
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    dut.mrst.value = 0
    await Timer(1, unit="ns")


@cocotb.test()
async def check_conditional_adds(dut):
    # Walk every enable combination a few times over; cnt must track
    # (cnt + 2*en1 + 3*en2) mod 8 and now_out must preview it combinationally.
    k = KSim(dut)
    await _reset(dut)
    exp = int(k.cnt.value)

    pattern = [(0, 0), (1, 0), (0, 1), (1, 1), (1, 1), (1, 0), (0, 1), (1, 1), (0, 0), (1, 1)]
    for step, (en1, en2) in enumerate(pattern):
        dut.en1_in.value = en1
        dut.en2_in.value = en2
        await Timer(1, unit="ns")                       # comb settle
        nxt = (exp + 2 * en1 + 3 * en2) % 8
        assert int(k.now_probe.value) == nxt, \
            f"step {step}: now_probe = {k.now_probe.value!s} (expected {nxt})"
        assert int(k.cnt.value) == exp, \
            f"step {step}: cnt moved before the edge: {k.cnt.value!s}"

        await RisingEdge(dut.clk)
        await Timer(1, unit="ns")
        exp = nxt
        assert int(k.cnt.value) == exp, \
            f"step {step}: cnt = {k.cnt.value!s} (expected {exp})"


@cocotb.test()
async def check_free_running_wrap(dut):
    # The enable-less counter adds 1 every cycle and wraps mod 16.
    k = KSim(dut)
    await _reset(dut)
    exp = int(k.free.value)

    for step in range(20):                              # > 16 → crosses the wrap
        await RisingEdge(dut.clk)
        await Timer(1, unit="ns")
        exp = (exp + 1) % 16
        assert int(k.free.value) == exp, \
            f"step {step}: free = {k.free.value!s} (expected {exp})"


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
