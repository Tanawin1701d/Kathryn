# tc35 — Karray MIXED-kind karray-to-karray copy: the three unified index kinds
# (static / dynamic binary / custom fn) in ONE statement, on BOTH sides:
#
#     a[1][en_fn][1][aw]  |=  b[max_fn][bw][1]
#         ^   ^    ^  ^          ^      ^   ^
#      static |  static|       reduce   |  static
#         cus enable  dyn (binary)     dyn (binary)
#
#   a shape (2, 3, 2, 3), b shape (2, 3, 2), element {data:8}, reg-backed.
# The write side guards with a custom enable on dim 1 (en_fn: element 1 only)
# AND a dynamic address on dim 3 (aw = const AW = 2); the read side folds b's
# dim 0 with a max-by-data reduce, then muxes dim 1 with bw = const BW = 1.
# So exactly ONE destination element takes max(b[0][1][1], b[1][1][1]).
# Two pre-written neighbours prove the guards: a[1][1][1][0] (aw enable low)
# and a[1][2][1][AW] (en_fn low) both keep their pre-written values.

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog
from kathryn.sim_assist import KSim

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc35_karray_mixed_k2k"

SETTLE_CYCLES = 60

B0   = 0x11      # b[0][1][1] — reduce candidate
B1   = 0x22      # b[1][1][1] — reduce candidate (the max -> the copied value)
PRE  = 0x77      # pre-written a[1][1][1][0]; must survive (aw enable low)
PRE2 = 0x55      # pre-written a[1][2][1][AW]; must survive (en_fn low)
AW   = 2         # a's dynamic write address (last dim, extent 3)
BW   = 1         # b's dynamic read address (middle dim, extent 3)
ENJ  = 1         # a's custom-enable target on dim 1 (extent 3)

EXPECT = {"o_k": max(B0, B1), "o_hold": PRE, "o_othr": PRE2}


# ---- model -------------------------------------------------------------------
class Cell(Karray):
    data = kaf(8)


class tc35_karray_mixed_k2k(Module):
    @init
    def com_declare(self):
        self.a = Cell(HwComponentType.REG, (2, 3, 2, 3), "a")
        self.b = Cell(HwComponentType.REG, (2, 3, 2), "b")

        self.aw    = val(2, AW,  "aw")       # binary address into a's dim 3
        self.bw    = val(2, BW,  "bw")       # binary address into b's dim 1
        self.ensel = val(2, ENJ, "ensel")    # custom-enable compare for a's dim 1

        self.o_k    = reg(8, "o_k")
        self.o_hold = reg(8, "o_hold")
        self.o_othr = reg(8, "o_othr")

    @flow
    def my_flow(self):
        for r in (self.o_k, self.o_hold, self.o_othr):
            r.reset(0)

        with seq():
            # fill the two reduce candidates the dynamic read will fold
            self.b[0][1][1] |= B0
            self.b[1][1][1] |= B1

            # pre-set the neighbours the two guards must NOT touch
            self.a[1][1][1][0]  |= PRE     # blocked by the dynamic (aw) enable
            self.a[1][2][1][AW] |= PRE2    # blocked by the custom (en_fn) enable

            # the mixed-kind copy under test
            self.a[1][lambda j: self.ensel == j][1][self.aw] |= \
                self.b[lambda x, y, l: x.fields["data"] >= y.fields["data"]][self.bw][1]

            # read back the copied element and both held neighbours
            self.o_k    |= self.a[1][ENJ][1][AW].data
            self.o_hold |= self.a[1][1][1][0].data
            self.o_othr |= self.a[1][2][1][AW].data


# ---- build (kathryn model -> verilog) ---------------------------------------
def build(output_folder: str) -> None:
    reset()
    build_model(tc35_karray_mixed_k2k())
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
@cocotb.test()
async def check_karray_mixed_k2k(dut):
    k = KSim(dut)
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value = 1
    await RisingEdge(dut.clk)
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    dut.mrst.value = 0

    for _ in range(SETTLE_CYCLES):
        await RisingEdge(dut.clk)
    await Timer(1, unit="ns")

    for name, want in EXPECT.items():
        got = int(getattr(k, name).value)
        assert got == want, f"{name} = {got} (expected {want})"


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
