# tc17 — reg reset values and wire default values, exercising the fallback events
# built during build_flow. Reset values are fed as DIRECT int literals (no manual
# val(...) wrapper), including one WIDER than 64 bits.
#
# Two fallback mechanisms:
#   * reg.reset(K)    — bound at DEFAULT_UE_PRI_RST (max), so it sorts LAST in the
#                       reg's clocked always-block and loads K every posedge.
#   * wire.default(s) — bound at internal-low priority; the wire takes s
#                       combinationally when nothing else drives it.
#
# NOTE on the wire default source: a constant-only combinational block
# (`always @(*) w <= CONST;`) has an empty sensitivity list, so Icarus never fires
# it and the wire stays X. The emitted Verilog is still correct (and the direct-int
# wire default is checked at the model/emit level in py/tests/test_smoke.py); here
# the default is sourced from a clocked reg so `@(*)` has a real net to track and
# the value is observable in simulation.

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog
from kathryn.sim_assist import KSim

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc17_reset_default"

# Direct int values fed straight to reset(...).
RESET_X   = 7
WIRE_SRC  = 9                                   # reset value of the wire-default source reg
WIDE_BITS = 72
WIDE_VAL  = (1 << 71) | (1 << 64) | 0xABCDEF    # spans the 2nd u64 limb + low bits


# ---- model -------------------------------------------------------------------
class tc17_reset_default(Module):
    @init
    def com_declare(self):
        # 8-bit reg whose reset value is the literal 7 (no val(...) needed).
        self.x = reg(8, "x")
        self.x.reset(RESET_X)

        # 72-bit reg reset from a >64-bit literal — exercises the u64-limb path.
        self.big = reg(WIDE_BITS, "big")
        self.big.reset(WIDE_VAL)

        # Clocked source for the wire default (reset to 9), then a wire defaulting
        # to it. A reg source keeps the wire's @(*) block sensitive to a real net.
        self.src = reg(8, "src")
        self.src.reset(WIRE_SRC)

        self.w = wire(8, "w")
        self.w.default(self.src)


# ---- build (kathryn model -> verilog) ---------------------------------------
def build(output_folder: str) -> None:
    reset()
    module = tc17_reset_default()
    build_model(module)
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
@cocotb.test()
async def check_reg_reset(dut):
    # reg.reset binds unconditionally at max priority, so my_x loads 7 on the first
    # posedge and holds it every cycle thereafter.
    k = KSim(dut)
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value = 1
    await RisingEdge(dut.clk)            # E1: first posedge — x <= 7
    await Timer(1, unit="ns")
    assert k.x.value == RESET_X, f"my_x = {k.x.value!s} (expected {RESET_X})"

    dut.mrst.value = 0
    for _ in range(10):                 # x is pinned to its reset value forever.
        await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert k.x.value == RESET_X, f"my_x drifted off {RESET_X}: {k.x.value!s}"


@cocotb.test()
async def check_wide_reset(dut):
    # A reset value wider than 64 bits is split into u64 limbs and reassembled; the
    # 72-bit reg must read back the exact literal after its first posedge.
    k = KSim(dut)
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value = 1
    await RisingEdge(dut.clk)           # E1: big <= WIDE_VAL
    await Timer(1, unit="ns")
    assert int(k.big.value) == WIDE_VAL, \
        f"my_big = {int(k.big.value):#x} (expected {WIDE_VAL:#x})"


@cocotb.test()
async def check_wire_default(dut):
    # wire.default(src) drives the wire combinationally from the source reg. Once
    # src latches its reset value (9) on the first posedge, the wire's @(*) block
    # fires and my_w tracks it.
    k = KSim(dut)
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value = 1
    await RisingEdge(dut.clk)           # src <= 9, wire follows
    await Timer(1, unit="ns")
    assert k.w.value == WIRE_SRC, f"my_w = {k.w.value!s} (expected {WIRE_SRC})"

    dut.mrst.value = 0
    for _ in range(5):                  # nothing else drives the wire — holds 9.
        await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert k.w.value == WIRE_SRC, f"my_w drifted off {WIRE_SRC}: {k.w.value!s}"


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
