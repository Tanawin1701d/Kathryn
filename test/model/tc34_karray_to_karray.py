# tc34 — karray-to-karray region assignment. Two reg-backed Karrays whose element
# layouts OVERLAP but differ:
#   src element = {valid:1, data:8, note:4}
#   dst element = {valid:1, data:8, tag:4}
# Fields are paired by exact name+width, so `valid` and `data` copy across while the
# non-matching `tag` (dst) / `note` (src) are skipped (a Python warning fires).
#
# The one-shot `seq` fills src, pre-sets dst[0].tag, then copies element by
# element (every selection names exactly one element):
#   * dst[0] |= src[1],  dst[1] |= src[2],  dst[3] |= src[0]
# tag is never copied, so dst[0].tag keeps its pre-set value — proving the skip.

from __future__ import annotations

import warnings

from kathryn import *
from kathryn import emit_verilog
from kathryn.sim_assist import KSim

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc34_karray_to_karray"

SETTLE_CYCLES = 50

TAG_PRESET = 0x5
# src[i] = (valid=1, data=DATA[i])
DATA = [0x11, 0x22, 0x33]

EXPECT = {
    "o_v0": 1,      "o_d0": DATA[1],   # dst[0] <- src[1]
    "o_v1": 1,      "o_d1": DATA[2],   # dst[1] <- src[2]
    "o_v3": 1,      "o_d3": DATA[0],   # dst[3] <- src[0]
    "o_tag0": TAG_PRESET,              # dst[0].tag untouched by the copies
}


# ---- model -------------------------------------------------------------------
class SrcArray(Karray):
    valid = kaf(1)
    data  = kaf(8)
    note  = kaf(4)


class DstArray(Karray):
    valid = kaf(1)
    data  = kaf(8)
    tag   = kaf(4)


class tc34_karray_to_karray(Module):
    @init
    def com_declare(self):
        self.src = SrcArray(HwComponentType.REG, (4,), "src")
        self.dst = DstArray(HwComponentType.REG, (4,), "dst")

        self.c_v   = val(1, 1,          "c_v")
        self.c_d0  = val(8, DATA[0],    "c_d0")
        self.c_d1  = val(8, DATA[1],    "c_d1")
        self.c_d2  = val(8, DATA[2],    "c_d2")
        self.c_tag = val(4, TAG_PRESET, "c_tag")

        self.o_v0   = reg(1, "o_v0")
        self.o_d0   = reg(8, "o_d0")
        self.o_v1   = reg(1, "o_v1")
        self.o_d1   = reg(8, "o_d1")
        self.o_v3   = reg(1, "o_v3")
        self.o_d3   = reg(8, "o_d3")
        self.o_tag0 = reg(4, "o_tag0")

    @flow
    def my_flow(self):
        for r in (self.o_v0, self.o_d0, self.o_v1, self.o_d1, self.o_v3, self.o_d3, self.o_tag0):
            r.reset(0)

        with seq():
            # fill the source entries used by the copies
            self.src[0].valid |= self.c_v; self.src[0].data |= self.c_d0
            self.src[1].valid |= self.c_v; self.src[1].data |= self.c_d1
            self.src[2].valid |= self.c_v; self.src[2].data |= self.c_d2

            # pre-set dst[0].tag so we can prove the copy leaves it alone
            self.dst[0].tag |= self.c_tag

            # element copies (valid+data matched; tag/note skipped -> warning)
            self.dst[0] |= self.src[1]
            self.dst[1] |= self.src[2]
            self.dst[3] |= self.src[0]

            # read back
            self.o_v0   |= self.dst[0].valid
            self.o_d0   |= self.dst[0].data
            self.o_v1   |= self.dst[1].valid
            self.o_d1   |= self.dst[1].data
            self.o_v3   |= self.dst[3].valid
            self.o_d3   |= self.dst[3].data
            self.o_tag0 |= self.dst[0].tag


# ---- build (kathryn model -> verilog) ---------------------------------------
def build(output_folder: str) -> None:
    reset()
    module = tc34_karray_to_karray()
    with warnings.catch_warnings(record=True) as caught:
        warnings.simplefilter("always")
        build_model(module)
    msgs = [str(w.message) for w in caught]
    tag_warns = [m for m in msgs if "tag" in m and "skipped" in m]
    assert tag_warns, f"expected a skipped-field warning mentioning 'tag', got {msgs}"

    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
@cocotb.test()
async def check_karray_to_karray(dut):
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
