# tc29 — Karray dynamic (runtime-signal) element read. A reg-backed register file
#   rf element = {valid:1, data:8}, shape (4,)
# is filled with known data, then read back through DYNAMIC indices that resolve to
# a balanced 2:1 mux tree (host `dynamic_index_get_Karray`):
#   * binary address  : rf[bsel_i].data for every i in 0..4  -> proves the binary
#                        decode picks the right element at all four addresses
#                        (bsel=2 = 0b10 exercises a non-zero MSB, not just index 0)
#   * one-hot select   : rf[oh(osel)].data with osel = 1<<2  -> index 2
# Selectors are constant `val`s, so each mux collapses to its selected element; the
# read still exercises the full decode wiring end to end through iverilog.

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc29_karray_dynamic_index"

SETTLE_CYCLES = 50

# rf[i] = (valid=1, data=DATA[i])
DATA = [0x11, 0x22, 0x33, 0x44]
OH_INDEX = 2                     # one-hot select picks this element

EXPECT = {f"d{i}": DATA[i] for i in range(4)}   # binary read of each address
EXPECT["od"] = DATA[OH_INDEX]                   # one-hot read of index 2


# ---- model -------------------------------------------------------------------
class RegFile(Karray):
    valid = kaf(1)
    data  = kaf(8)


class tc29_karray_dynamic_index(Module):
    @init
    def com_declare(self):
        self.rf = RegFile(HwComponentType.REG, (4,), "rf")

        self.c_v = val(1, 1, "c_v")
        self.c_d = [val(8, DATA[i], f"c_d{i}") for i in range(4)]

        # constant index selectors: a binary address per element, plus one one-hot
        self.bsel = [val(2, i, f"bsel{i}") for i in range(4)]
        self.osel = val(4, 1 << OH_INDEX, "osel")

        # one output per binary read, plus the one-hot read
        self.o_d = [reg(8, f"o_d{i}") for i in range(4)]
        for i in range(4):
            self.o_d[i].mark_output(f"d{i}")
        self.o_od = reg(8, "o_od"); self.o_od.mark_output("od")

    @flow
    def my_flow(self):
        for r in (*self.o_d, self.o_od):
            r.reset(0)

        with seq():
            # fill every element with its known (valid, data)
            for i in range(4):
                self.rf[i].valid |= self.c_v
                self.rf[i].data  |= self.c_d[i]

            # binary dynamic read of each address (rf[bsel_i].data == DATA[i])
            for i in range(4):
                self.o_d[i] |= self.rf[self.bsel[i]].data

            # one-hot dynamic read (rf[oh(osel)].data == DATA[OH_INDEX])
            self.o_od |= self.rf[oh(self.osel)].data


# ---- build (kathryn model -> verilog) ---------------------------------------
def build(output_folder: str) -> None:
    reset()
    build_model(tc29_karray_dynamic_index())
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
@cocotb.test()
async def check_karray_dynamic_index(dut):
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
