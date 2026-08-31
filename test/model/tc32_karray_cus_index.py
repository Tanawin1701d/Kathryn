# tc32 — Karray CUSTOM-FN index (the unified replacement for cus_dynamic_assign)
# plus int-literal sources and element map writes.
#   rf/rg element = {data:8}, shape (4,), reg-backed
# A callable index fans its dimension out with one user-built 1-bit enable per
# index (`fn(i) -> enable`); a write lands only where the enable is high, every
# other element holds:
#   * compare enables : rf[lambda j: seli == j].data |= D[i] with seli = const i
#                       -> element i = D[i], for all four i (distinct values prove
#                          the enables route writes without cross-clobber)
#   * map writes      : rg[0] / rg[1] |= {data: SENTINEL} overwrite elements 0,1
#                       only after the same seeding (2,3 keep their seed)
#   * int source      : rh[msel] |= INTSRC (msel = const 3) — a dynamic write whose
#                       source is a raw int, wrapped into a field-width val
#   * reduce read     : o_rd |= rf[max_fn].data — on the READ side a custom fn is
#                       a reduce pair-select, so this folds rf to its max element

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog
from kathryn.sim_assist import KSim

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc32_karray_cus_index"

SETTLE_CYCLES = 50

DATA     = [0x10, 0x21, 0x32, 0x43]   # per-element custom-enable write
SENTINEL = 0x7E                       # range broadcast into elements 0,1
INTSRC   = 0x5A                       # int-literal source through a dynamic write

EXPECT = {f"o_d[{i}]": DATA[i] for i in range(4)}
EXPECT["o_g[0]"] = SENTINEL
EXPECT["o_g[1]"] = SENTINEL
EXPECT["o_g[2]"] = DATA[2]             # rg seeded per-element first; 2,3 keep it
EXPECT["o_g[3]"] = DATA[3]
EXPECT["o_h3"] = INTSRC
EXPECT["o_rd"] = max(DATA)             # reduce read folds rf to its max element


# ---- model -------------------------------------------------------------------
class RegFile(Karray):
    data = kaf(8)


class tc32_karray_cus_index(Module):
    @init
    def com_declare(self):
        self.rf = RegFile(HwComponentType.REG, (4,), "rf")
        self.rg = RegFile(HwComponentType.REG, (4,), "rg")
        self.rh = RegFile(HwComponentType.REG, (4,), "rh")

        self.c_d  = [val(8, DATA[i], f"c_d{i}") for i in range(4)]
        self.seli = [val(2, i, f"seli{i}") for i in range(4)]
        self.msel = val(2, 3, "msel")             # binary address -> element 3 (int-source write)

        # read the regfiles' elements back to outputs
        self.o_d  = [reg(8, f"o_d{i}") for i in range(4)]
        self.o_g  = [reg(8, f"o_g{i}") for i in range(4)]
        self.o_h3 = reg(8, "o_h3")
        self.o_rd = reg(8, "o_rd")

    @flow
    def my_flow(self):
        for r in (*self.o_d, *self.o_g, self.o_h3, self.o_rd):
            r.reset(0)

        with seq():
            # per-element custom enables: element j enabled iff seli == j
            for i in range(4):
                self.rf[lambda j, i=i: self.seli[i] == j].data |= self.c_d[i]
                # seed rg the same way so its elements 2,3 have a known value
                self.rg[lambda j, i=i: self.seli[i] == j].data |= self.c_d[i]

            # element map writes: elements 0 and 1 only (2,3 untouched)
            self.rg[0] |= {"data": SENTINEL}
            self.rg[1] |= {"data": SENTINEL}

            # dynamic write with an int-literal source (wrapped to the field width)
            self.rh[self.msel] |= INTSRC

            # reduce read: fold rf with a max-by-data pair select
            self.o_rd |= self.rf[lambda a, b, l: a.fields["data"] >= b.fields["data"]].data

            for i in range(4):
                self.o_d[i] |= self.rf[i].data
                self.o_g[i] |= self.rg[i].data
            self.o_h3 |= self.rh[3].data


# ---- build (kathryn model -> verilog) ---------------------------------------
def build(output_folder: str) -> None:
    reset()
    build_model(tc32_karray_cus_index())
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
@cocotb.test()
async def check_karray_cus_index(dut):
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

    sigs = {f"o_d[{i}]": k.o_d[i] for i in range(4)}
    sigs.update({f"o_g[{i}]": k.o_g[i] for i in range(4)})
    sigs["o_h3"] = k.o_h3
    sigs["o_rd"] = k.o_rd
    for name, want in EXPECT.items():
        got = int(sigs[name].value)
        assert got == want, f"{name} = {got} (expected {want})"


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
