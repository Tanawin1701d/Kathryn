# tc11 — wait blocks: sywait (fixed cycle wait) and scwait (condition wait).
# A sequence latches x<=1, stalls on a wait block, then latches x<=2; the wait
# block delays the second assignment.

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc11_wait"


# ---- model -------------------------------------------------------------------
class tc11_wait(Module):
    @init
    def com_declare(self):
        self.x     = reg (8, "x")
        self.go    = wire(1, "go")        # 1-bit input for scwait
        self.one   = val (8, 1, "one")
        self.two   = val (8, 2, "two")
        self.three = val (8, 3, "three")

        self.go.mark_input("go_in")
        self.x.mark_output("my_x")

    @flow
    def my_flow(self):
        with seq():
            self.x |= self.one          # x <= 1
            sywait(3)                   # stall 3 clocks (no input needed)
            self.x |= self.two          # x <= 2  (only after sywait completes)
            scwait(self.go)             # stall until go == 1
            self.x |= self.three        # x <= 3  (only after go)


# ---- build -------------------------------------------------------------------
def build(output_folder: str) -> None:
    reset()
    module = tc11_wait()
    build_model(module)
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
@cocotb.test()
async def check_wait(dut):
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value  = 1
    dut.go_in.value = 0
    await RisingEdge(dut.clk)            # E1
    await RisingEdge(dut.clk)            # E2
    await Timer(1, unit="ns")
    dut.mrst.value = 0

    # sywait runs on its own (no input). With go held low, the flow latches
    # x<=1, stalls 3 clocks in sywait, latches x<=2, then blocks in scwait.
    # After plenty of clocks x must sit at 2 — past sywait, stuck on scwait.
    for _ in range(12):
        await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert dut.my_x.value == 2, f"sywait/scwait wrong: expected x stuck at 2, got {dut.my_x.value!s}"

    # release the condition wait → x advances to 3.
    dut.go_in.value = 1
    for _ in range(6):
        await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert dut.my_x.value == 3, f"x did not reach 3 after go=1: {dut.my_x.value!s}"


@cocotb.test()
async def check_sywait_delays(dut):
    # Focused sywait check: x must hold at 1 for the sywait stall before it can
    # become 2, proving the cycle wait actually delays the next assignment.
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value  = 1
    dut.go_in.value = 1
    await RisingEdge(dut.clk)
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    dut.mrst.value = 0

    # Walk cycle-by-cycle: once x reaches 1 it must stay 1 for at least the
    # 3-cycle sywait stall before advancing to 2.
    saw_one      = False
    one_streak   = 0
    advanced_to2 = False
    for _ in range(14):
        await RisingEdge(dut.clk)
        await Timer(1, unit="ns")
        if not dut.my_x.value.is_resolvable:    # x still settling after reset
            continue
        v = int(dut.my_x.value)
        if v == 1:
            saw_one = True
            one_streak += 1
        elif v == 2 and saw_one:
            advanced_to2 = True
            break
        assert v in (0, 1, 2), f"unexpected x={v}"

    assert saw_one,        "x never latched 1"
    assert advanced_to2,   "x never advanced to 2 after sywait"
    assert one_streak == 4, f"sywait must hold one_strek for 4 cycles"
    await RisingEdge(dut.clk)


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
