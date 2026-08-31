# tc19 — assignment-source auto-resize. When a source's width does not match the
# destination region, the Rust connector sanitizes it before building the asm:
#
#   src narrower than dest  -> zero-extended (unsigned): high bits become 0
#   src wider   than dest   -> MSBs dropped:              only the low bits land
#
# Each implicit resize raises a Python `warnings.warn` back to the DSL; build()
# captures them and asserts the expected messages fired, then the cocotb run
# confirms the simulated values match the masked semantics.

from __future__ import annotations

import warnings

from kathryn import *
from kathryn import emit_verilog
from kathryn.sim_assist import KSim

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc19_asm_resize"

WIDE   = 0xAB   # 8-bit pinned source (1010_1011)
NARROW = 0xD    # 4-bit pinned source (1101)

EXPECT = {
    # 4-bit dest, 8-bit source -> keep low 4 bits of 0xAB = 0xB
    "trunc":  WIDE & 0xF,        # 11
    # 8-bit dest, 4-bit source -> zero-extend 0xD = 0x0D
    "extend": NARROW & 0xFF,     # 13
    # control: exact width match -> untouched
    "exact":  WIDE & 0xFF,       # 171
}


# ---- model -------------------------------------------------------------------
class tc19_asm_resize(Module):
    @init
    def com_declare(self):
        self.wide   = reg(8, "wide");   self.wide.reset(WIDE)
        self.narrow = reg(4, "narrow"); self.narrow.reset(NARROW)

        self.trunc  = reg(4, "trunc_r")
        self.extend = reg(8, "extend_r")
        self.exact  = reg(8, "exact_r")

    @flow
    def my_flow(self):
        with seq():
            self.trunc  |= self.wide      # 8 -> 4 : drop MSBs (warns)
            self.extend |= self.narrow    # 4 -> 8 : zero-extend (warns)
            self.exact  |= self.wide      # 8 -> 8 : untouched (no warn)


# ---- build (kathryn model -> verilog) ---------------------------------------
def build(output_folder: str) -> None:
    reset()
    module = tc19_asm_resize()
    with warnings.catch_warnings(record=True) as caught:
        warnings.simplefilter("always")
        build_model(module)
    msgs = [str(w.message) for w in caught]

    # Exactly the two mismatched assignments must have warned.
    extend_warns = [m for m in msgs if "zero-extended" in m]
    trunc_warns  = [m for m in msgs if "MSBs dropped"  in m]
    assert len(extend_warns) == 1, f"expected 1 zero-extend warning, got {msgs}"
    assert len(trunc_warns)  == 1, f"expected 1 truncate warning, got {msgs}"
    # The exact-width assignment must not have warned.
    assert all("exact" not in m for m in msgs), f"exact-width assign should not warn: {msgs}"

    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
@cocotb.test()
async def check_asm_resize(dut):
    k = KSim(dut)
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value = 1
    await RisingEdge(dut.clk)            # E1: start <= 1
    await RisingEdge(dut.clk)            # E2: start held
    await Timer(1, unit="ns")
    dut.mrst.value = 0

    for _ in range(20):
        await RisingEdge(dut.clk)
    await Timer(1, unit="ns")

    for name, want in EXPECT.items():
        got = int(getattr(k, name).value)
        assert got == want, f"{name} = {got} (expected {want})"


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
