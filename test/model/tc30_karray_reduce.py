# tc30 — Karray callback-driven reduce. A reg-backed register file
#   rf element = {valid:1, data:8}, shape (4,)
# is filled with known data, then reduced to a single winner by a USER select
# function called per compared pair (host `reduce_*` / connector `karray_dynamic_reduce_get`):
#   * pick_max   : a.data >= b.data                       -> element with max data
#   * pick_valid : a.valid & (~b.valid | a.data>=b.data)  -> max data AMONG VALID ones
# pick_valid proves the callback consumes BOTH fields: the global max (0x99) is
# invalid, so the valid-gated reduce must instead pick the max valid element (0x55),
# not the raw max. The select expressions become combinational logic over the regs.

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc30_karray_reduce"

SETTLE_CYCLES = 50

# rf[i] = (valid=VALID[i], data=DATA[i])
DATA  = [0x55, 0x44, 0x99, 0x22]
VALID = [1,    1,    0,    1]

EXPECT = {
    "dm": 0x99,   # plain max data (ignores valid)            -> element 2
    "dv": 0x55,   # max data among VALID elements             -> element 0
    "vv": 1,      # the valid-gated winner is itself valid
}


# ---- select functions (called back per compared pair) ------------------------
def pick_max(a, b, level):
    return a.fields["data"] >= b.fields["data"]


def pick_valid(a, b, level):
    av, bv = a.fields["valid"], b.fields["valid"]
    ad, bd = a.fields["data"],  b.fields["data"]
    # a wins if a is valid AND (b is invalid OR a.data >= b.data)
    return av & ((~bv) | (ad >= bd))


# ---- model -------------------------------------------------------------------
class RegFile(Karray):
    valid = kaf(1)
    data  = kaf(8)


class tc30_karray_reduce(Module):
    @init
    def com_declare(self):
        self.rf = RegFile(HwComponentType.REG, (4,), "rf")

        self.c_d = [val(8, DATA[i],  f"c_d{i}") for i in range(4)]
        self.c_v = [val(1, VALID[i], f"c_v{i}") for i in range(4)]

        self.o_dm = reg(8, "o_dm"); self.o_dm.mark_output("dm")
        self.o_dv = reg(8, "o_dv"); self.o_dv.mark_output("dv")
        self.o_vv = reg(1, "o_vv"); self.o_vv.mark_output("vv")

    @flow
    def my_flow(self):
        for r in (self.o_dm, self.o_dv, self.o_vv):
            r.reset(0)

        with seq():
            # fill every element with its known (valid, data)
            for i in range(4):
                self.rf[i].valid |= self.c_v[i]
                self.rf[i].data  |= self.c_d[i]

            # reduce to the max-data element, and to the max-data-among-valid element
            w_max   = self.rf.reduce([Reduce], pick_max)
            w_valid = self.rf.reduce([Reduce], pick_valid)

            self.o_dm |= w_max.data
            self.o_dv |= w_valid.data
            self.o_vv |= w_valid.valid


# ---- build (kathryn model -> verilog) ---------------------------------------
def build(output_folder: str) -> None:
    reset()
    build_model(tc30_karray_reduce())
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
@cocotb.test()
async def check_karray_reduce(dut):
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
