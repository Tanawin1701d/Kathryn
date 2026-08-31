# tc31 — Karray dynamic (runtime-signal) element WRITE (the mirror of tc30's read).
#   rf element = {valid:1, data:8}, shape (4,), reg-backed
# Each element is written through a DYNAMIC index so the per-element write-enable
# decode (host `dynamic_assign_hcps`) is exercised end to end:
#   * binary address : rf[bsel0].data |= D0 and rf[bsel2].data |= D2 (bsel = const 0/2)
#                      -> proves the (sel == k) enable routes the write to one element
#                         (and only that one — element 1 / 3 are written by the other
#                          mechanisms, never clobbered by the binary writes)
#   * custom fn (one-hot style): rf[lambda i: osel[i]].data |= DOH, osel = 1<<1 -> element 1
#   * whole-element  : rf[msel] |= {valid:1, data:DMAP} with msel = const 3 -> element 3
# Each element is written by exactly one statement (distinct values), so the final
# state is deterministic regardless of the seq state machine looping: a write only
# lands where its enable is high, every other element holds.

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog
from kathryn.sim_assist import KSim

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc31_karray_dynamic_assign"

SETTLE_CYCLES = 50

D0   = 0x11      # binary write -> element 0
D2   = 0x33      # binary write -> element 2
DOH  = 0xAB      # one-hot write -> element 1
DMAP = 0xCD      # map write     -> element 3
MAP_V = 1

EXPECT = {"o_d[0]": D0, "o_d[1]": DOH, "o_d[2]": D2, "o_d[3]": DMAP, "o_v3": MAP_V}


# ---- model -------------------------------------------------------------------
class RegFile(Karray):
    valid = kaf(1)
    data  = kaf(8)


class tc31_karray_dynamic_assign(Module):
    @init
    def com_declare(self):
        self.rf = RegFile(HwComponentType.REG, (4,), "rf")

        # write data + constant index selectors
        self.c_d0  = val(8, D0,  "c_d0")
        self.c_d2  = val(8, D2,  "c_d2")
        self.c_doh = val(8, DOH, "c_doh")
        self.c_dmp = val(8, DMAP, "c_dmp")
        self.c_mv  = val(1, MAP_V, "c_mv")

        self.bsel0 = val(2, 0, "bsel0")          # binary address -> element 0
        self.bsel2 = val(2, 2, "bsel2")          # binary address -> element 2
        self.osel  = val(4, 1 << 1, "osel")      # one-hot        -> element 1
        self.msel  = val(2, 3, "msel")           # binary address -> element 3 (map)

        # read every element's data back to an output, plus element 3's valid
        self.o_d = [reg(8, f"o_d{i}") for i in range(4)]
        self.o_v3 = reg(1, "o_v3")

    @flow
    def my_flow(self):
        for r in (*self.o_d, self.o_v3):
            r.reset(0)

        with seq():
            # dynamic writes — each lands on exactly one element via its enable
            self.rf[self.bsel0].data |= self.c_d0           # binary  -> element 0
            self.rf[self.bsel2].data |= self.c_d2           # binary  -> element 2
            self.rf[lambda i: self.osel[i]].data |= self.c_doh   # custom fn (one-hot) -> element 1
            self.rf[self.msel] |= {"valid": self.c_mv, "data": self.c_dmp}  # map -> element 3

            # static read-back of the final element state
            for i in range(4):
                self.o_d[i] |= self.rf[i].data
            self.o_v3 |= self.rf[3].valid


# ---- build (kathryn model -> verilog) ---------------------------------------
def build(output_folder: str) -> None:
    reset()
    build_model(tc31_karray_dynamic_assign())
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
@cocotb.test()
async def check_karray_dynamic_assign(dut):
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
    sigs["o_v3"] = k.o_v3
    for name, want in EXPECT.items():
        got = int(sigs[name].value)
        assert got == want, f"{name} = {got} (expected {want})"


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
