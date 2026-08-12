# tc33 — Karray REDUCE read (the read-side custom-fn index): a callable index
# folds its dimension through a balanced 2:1 reduce tree; the user's select fn
# is called per compared pair (`fn(a, b, level) -> pick-a` over ReduceViews) and
# may carry extras (`(select, {name: signal})` — an extra replaces a same-named
# field on the merged node, so it is what the next level and the result see).
#   * max     : o_max |= rf[pick_max].data                 -> max(DATA)
#   * extras  : o_sum |= rf[pick_sum].data, where the fn replaces "data" with
#               a_data + b_data at every level              -> sum(DATA)
#   * 2-D     : o_r1  |= grid[1][pick_max].d  (pin row 1, fold cols)
#                                                           -> max(ROW1)

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc33_karray_reduce_read"

SETTLE_CYCLES = 50

DATA = [0x01, 0x02, 0x03, 0x04]       # rf elements
ROW0 = [0x05, 0x01, 0x03]             # grid row 0
ROW1 = [0x02, 0x06, 0x04]             # grid row 1

EXPECT = {
    "mx": max(DATA),                   # 0x04
    "sm": sum(DATA),                   # 0x0A
    "r1": max(ROW1),                   # 0x06
}


def pick_max_data(a, b, level):
    return a.fields["data"] >= b.fields["data"]


def pick_sum_data(a, b, level):
    asum = a.fields["data"]
    bsum = b.fields["data"]
    return (asum >= bsum), {"data": asum + bsum}   # carried data = running sum


def pick_max_d(a, b, level):
    return a.fields["d"] >= b.fields["d"]


# ---- model -------------------------------------------------------------------
class RegFile(Karray):
    data = kaf(8)


class Cell(Karray):
    d = kaf(6)


class tc33_karray_reduce_read(Module):
    @init
    def com_declare(self):
        self.rf   = RegFile(HwComponentType.REG, (4,), "rf")
        self.grid = Cell(HwComponentType.REG, (2, 3), "grid")

        self.o_mx = reg(8, "o_mx"); self.o_mx.mark_output("mx")
        self.o_sm = reg(8, "o_sm"); self.o_sm.mark_output("sm")
        self.o_r1 = reg(6, "o_r1"); self.o_r1.mark_output("r1")

    @flow
    def my_flow(self):
        for r in (self.o_mx, self.o_sm, self.o_r1):
            r.reset(0)

        with seq():
            # fill the arrays with known values (int literals auto-wrap)
            for i in range(4):
                self.rf[i] |= DATA[i]                       # single-field -> sole field
            for c in range(3):
                self.grid[0][c] |= ROW0[c]
                self.grid[1][c] |= ROW1[c]

            # reduce reads
            self.o_mx |= self.rf[pick_max_data].data
            self.o_sm |= self.rf[pick_sum_data].data
            self.o_r1 |= self.grid[1][pick_max_d].d


# ---- build (kathryn model -> verilog) ---------------------------------------
def build(output_folder: str) -> None:
    reset()
    build_model(tc33_karray_reduce_read())
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
@cocotb.test()
async def check_karray_reduce_read(dut):
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
