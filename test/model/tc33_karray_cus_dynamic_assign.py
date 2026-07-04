# tc33 — Karray cus_dynamic_assign: custom callback-driven dynamic WRITE.
#   rf element = {data:8}, shape (4,), reg-backed
# The user's write fn builds each element's write-enable from the element's STATIC
# coordinate (exposed as `view.coord`) and a closed-over runtime index signal:
#   * per-element : for each i, cus_dynamic_assign([Spread], {data:D[i]},
#                   we = (sel_i == coord[0]) with sel_i = const i)  -> element i = D[i].
#                   Only the matching element's enable is high, every other holds —
#                   so writing all four with distinct values proves the callback path
#                   and coord exposure route writes correctly and never cross-clobber.
#   * range : one extra write over a dim-level slice [0, THR) (THR = 2) with an
#             always-high write-enable drives a SENTINEL into elements 0 and 1 only —
#             so the RANGE itself (not a callback filter) restricts the fan-out —
#             into a separate regfile so the per-element checks stay deterministic.

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc33_karray_cus_dynamic_assign"

SETTLE_CYCLES = 50

DATA     = [0x10, 0x21, 0x32, 0x43]   # per-element custom-we write
SENTINEL = 0x7E                       # range write: elements where 2 > coord
THR      = 2

# rf: per-element writes -> D[i]. rg: range write -> elements 0,1 = SENTINEL, 2,3 hold.
EXPECT = {f"d{i}": DATA[i] for i in range(4)}
EXPECT["g0"] = SENTINEL
EXPECT["g1"] = SENTINEL
EXPECT["g2"] = DATA[2]                 # rg also seeded per-element first, 2,3 keep it
EXPECT["g3"] = DATA[3]


# ---- model -------------------------------------------------------------------
class RegFile(Karray):
    data = kaf(8)


class tc33_karray_cus_dynamic_assign(Module):
    @init
    def com_declare(self):
        self.rf = RegFile(HwComponentType.REG, (4,), "rf")
        self.rg = RegFile(HwComponentType.REG, (4,), "rg")

        self.c_d  = [val(8, DATA[i], f"c_d{i}") for i in range(4)]
        self.seli = [val(2, i, f"seli{i}") for i in range(4)]
        self.c_sn = val(8, SENTINEL, "c_sn")
        self.thr  = val(8, THR, "thr")
        self.we_hi = val(1, 1, "we_hi")           # always-high enable for the range write

        # read both regfiles' elements back to outputs
        self.o_d = [reg(8, f"o_d{i}") for i in range(4)]
        self.o_g = [reg(8, f"o_g{i}") for i in range(4)]
        for i in range(4):
            self.o_d[i].mark_output(f"d{i}")
            self.o_g[i].mark_output(f"g{i}")

    @flow
    def my_flow(self):
        for r in (*self.o_d, *self.o_g):
            r.reset(0)

        with seq():
            # per-element custom write-enable: element i enabled iff seli == coord[0]
            for i in range(4):
                self.rf.cus_dynamic_assign(
                    [Spread], {"data": self.c_d[i]},
                    lambda v, i=i: self.seli[i] == v.coord[0],
                )
                # seed rg the same way so its elements 2,3 have a known value
                self.rg.cus_dynamic_assign(
                    [Spread], {"data": self.c_d[i]},
                    lambda v, i=i: self.seli[i] == v.coord[0],
                )

            # dim-level range: fan out only over cols [0, THR) with an always-high
            # write-enable, so the RANGE restricts the write to cols 0,1 (2,3 untouched)
            self.rg.cus_dynamic_assign(
                [slice(0, THR)], {"data": self.c_sn},
                lambda v: self.we_hi,
            )

            for i in range(4):
                self.o_d[i] |= self.rf[i].data
                self.o_g[i] |= self.rg[i].data


# ---- build (kathryn model -> verilog) ---------------------------------------
def build(output_folder: str) -> None:
    reset()
    build_model(tc33_karray_cus_dynamic_assign())
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
@cocotb.test()
async def check_karray_cus_dynamic_assign(dut):
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
