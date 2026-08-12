# tc44 — Karray BUNDLES (nested kaf, Chisel-Bundle style), end to end.
#   Vec2  = {x:8, y:8}                 (a KBundle)
#   Entry = {valid:1, pos: kaf(Vec2)}  -> flat leaf fields valid, pos_x, pos_y
# Exercised:
#   * whole-element write via NESTED dicts: a[0] |= {valid:1, pos:{x:X0, y:Y0}}
#   * bundle-FIELD map write:              a[1].pos |= {x:X1, y:Y1}
#   * leaf write through the attr chain:   a[1].valid |= 1
#   * k2k with structural bundle pairing:  b[0] |= a[0]  (flat name+width match)
# Read-back goes through the same chained attributes (a[i].pos.x).

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc44_karray_bundle"

SETTLE_CYCLES = 50

X0, Y0 = 0x12, 0x34               # a[0].pos via nested dict
X1, Y1 = 0x56, 0x07               # a[1].pos via bundle-field map

EXPECT = {
    "ax0": X0, "ay0": Y0,          # a[0].pos.{x,y}
    "ax1": X1, "ay1": Y1,          # a[1].pos.{x,y}
    "av1": 1,                      # a[1].valid
    "bx0": X0, "by0": Y0, "bv0": 1,   # b[0] <- a[0] (bundle k2k)
}


# ---- model -------------------------------------------------------------------
class Vec2(KBundle):
    x = kaf(8)
    y = kaf(8)


class Entry(Karray):
    valid = kaf(1)
    pos   = kaf(Vec2)


class tc44_karray_bundle(Module):
    @init
    def com_declare(self):
        self.a = Entry(HwComponentType.REG, (2,), "a")
        self.b = Entry(HwComponentType.REG, (2,), "b")

        self.o = {}
        for name, width in (("ax0", 8), ("ay0", 8), ("ax1", 8), ("ay1", 8),
                            ("av1", 1), ("bx0", 8), ("by0", 8), ("bv0", 1)):
            r = reg(width, f"o_{name}")
            r.mark_output(name)
            self.o[name] = r

    @flow
    def my_flow(self):
        for r in self.o.values():
            r.reset(0)

        with seq():
            # whole element via nested dicts (int literals auto-wrap per field)
            self.a[0] |= {"valid": 1, "pos": {"x": X0, "y": Y0}}

            # bundle-field map + leaf write through the chain
            self.a[1].pos |= {"x": X1, "y": Y1}
            self.a[1].valid |= 1

            # k2k: bundles pair structurally by flat name+width
            self.b[0] |= self.a[0]

            # read back through the chained attributes
            self.o["ax0"] |= self.a[0].pos.x
            self.o["ay0"] |= self.a[0].pos.y
            self.o["ax1"] |= self.a[1].pos.x
            self.o["ay1"] |= self.a[1].pos.y
            self.o["av1"] |= self.a[1].valid
            self.o["bx0"] |= self.b[0].pos.x
            self.o["by0"] |= self.b[0].pos.y
            self.o["bv0"] |= self.b[0].valid


# ---- build (kathryn model -> verilog) ---------------------------------------
def build(output_folder: str) -> None:
    reset()
    build_model(tc44_karray_bundle())
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
@cocotb.test()
async def check_karray_bundle(dut):
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
