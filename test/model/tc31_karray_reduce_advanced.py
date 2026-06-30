# tc31 — advanced Karray reduce: per-dimension select functions (nested), the
# resolved per-dimension winner index (request_index), and intermediate-wire
# extras carried between layers.
#
#   grid (2,3) {data}: reduce cols by MAX, then rows by MIN (nested, per-dim fns)
#     row0 = [0x10, 0x40, 0x20] -> col-max 0x40 @ col1
#     row1 = [0x55, 0x33, 0x11] -> col-max 0x55 @ col0
#     row-min(0x40, 0x55)      -> 0x40, winner coord (row0, col1)
#   so winner.data = 0x40, request_index coords = [row=0, col=1].
#
#   rf (4,) {data}: extras carry a running SUM; the select compares running sums.
#     data = [0x10, 0x20, 0x05, 0x08]
#     L0: (e0,e1) sums 0x10,0x20 -> pick e1 (0x20), runsum 0x30
#         (e2,e3) sums 0x05,0x08 -> pick e3 (0x08), runsum 0x0D
#     L1: 0x30 >= 0x0D -> pick the (e0,e1) winner -> 0x20
#   so the running-sum-select winner.data = 0x20 (proves extras reach the next layer).

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc31_karray_reduce_advanced"

SETTLE_CYCLES = 50

GRID = [[0x10, 0x40, 0x20],
        [0x55, 0x33, 0x11]]
RF   = [0x10, 0x20, 0x05, 0x08]

EXPECT = {
    "gd":  0x40,   # nested col-max then row-min winner data
    "grow": 0,     # winner row index
    "gcol": 1,     # winner col index
    "rd":  0x20,   # running-sum-select winner data (extras)
}


# ---- select functions --------------------------------------------------------
def col_max(a, b, level):
    return a.fields["data"] >= b.fields["data"]


def row_min(a, b, level):
    return a.fields["data"] <= b.fields["data"]


def pick_sum(a, b, level):
    asum = a.fields.get("runsum", a.fields["data"])   # seed from data at the leaves
    bsum = b.fields.get("runsum", b.fields["data"])
    return (asum >= bsum), {"runsum": asum + bsum}


# ---- model -------------------------------------------------------------------
class Cell(Karray):
    data = kaf(8)


class Rf(Karray):
    data = kaf(8)


class tc31_karray_reduce_advanced(Module):
    @init
    def com_declare(self):
        self.grid = Cell(HwComponentType.REG, (2, 3), "grid")
        self.rf   = Rf  (HwComponentType.REG, (4,),   "rf")

        self.c_g = [[val(8, GRID[r][c], f"c_g{r}_{c}") for c in range(3)] for r in range(2)]
        self.c_r = [val(8, RF[i], f"c_r{i}") for i in range(4)]

        self.o_gd   = reg(8, "o_gd");   self.o_gd  .mark_output("gd")
        self.o_grow = reg(1, "o_grow"); self.o_grow.mark_output("grow")
        self.o_gcol = reg(2, "o_gcol"); self.o_gcol.mark_output("gcol")
        self.o_rd   = reg(8, "o_rd");   self.o_rd  .mark_output("rd")

    @flow
    def my_flow(self):
        for r in (self.o_gd, self.o_grow, self.o_gcol, self.o_rd):
            r.reset(0)

        with seq():
            # fill both arrays
            for r in range(2):
                for c in range(3):
                    self.grid[r][c].data |= self.c_g[r][c]
            for i in range(4):
                self.rf[i].data |= self.c_r[i]

            # nested per-dim reduce (rows: min, cols: max) + per-dim winner coords
            gwin, gcoords = self.grid.reduce([Reduce(row_min), Reduce(col_max)], request_index=True)
            self.o_gd   |= gwin.data
            self.o_grow |= gcoords[0]      # winner row index
            self.o_gcol |= gcoords[1]      # winner col index

            # extras: running-sum-select over the 1-D array
            rwin = self.rf.reduce([Reduce], pick_sum)
            self.o_rd |= rwin.data


# ---- build (kathryn model -> verilog) ---------------------------------------
def build(output_folder: str) -> None:
    reset()
    build_model(tc31_karray_reduce_advanced())
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
@cocotb.test()
async def check_karray_reduce_advanced(dut):
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value = 1
    await RisingEdge(dut.clk)
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    dut.mrst.value = 0

    for _ in range(SETTLE_CYCLES):
        await RisingEdge(dut.clk)
    await Timer(1, unit="ns")

    for port, want in EXPECT.items():
        got = int(getattr(dut, port).value)
        assert got == want, f"{port} = {got} (expected {want})"


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
