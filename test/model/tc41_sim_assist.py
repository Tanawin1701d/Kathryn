# tc41 — sim assist: observe and force INTERNAL signals through KSim, with no
# mark_output port. Covers every manifest node kind: plain reg, sliced view,
# counter, karray element field, and a sub-module's internal reg; plus a
# combinational wire that proves a forced value propagates downstream.

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog
from kathryn.complex_hardware.counter import counter
from kathryn.sim_assist import KSim

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc41_sim_assist"


# ---- model -------------------------------------------------------------------
class tc41_child(Module):
    @init
    def com_declare(self):
        self.acc = reg(8, "acc")

    @flow
    def my_flow(self):
        with seq():
            self.acc |= 7


class tc41_rf(Karray):
    data = kaf(8)


class tc41_sim_assist(Module):
    @init
    def com_declare(self):
        self.x   = reg (8, "x")
        self.w   = wire(8, "w")           # comb mirror of x+1 — downstream of a force
        self.w2  = wire(8, "w2")          # comb reader of rf[1] — an unread reg would be
                                          # elided from the sim hierarchy by icarus
        self.hi  = self.x[7, 4]           # sliced VIEW of x (no net of its own)
        self.cnt = counter(8, "c")
        self.rf  = tc41_rf(HwComponentType.REG, (2,), "rf")
        self.sub = tc41_child()
        self.cnt.reset(0)
        # NO mark_output anywhere — everything below is read through KSim.

    @flow
    def my_flow(self):
        self.w  *= self.x + 1             # bare comb node: always @(*) w = x + 1
        self.w2 *= self.rf[1].data        # static karray read keeps rf[1] alive in the sim
        with seq():
            self.x |= 0xA5                # hi nibble 0xA for the slice check
            self.cnt.add(5)
            self.cnt.update()
            self.rf[0] |= 3               # rf[1] left unwritten (force target)


# ---- build -------------------------------------------------------------------
def build(output_folder: str) -> None:
    reset()
    module = tc41_sim_assist()
    build_model(module)
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
async def _reset_and_run(dut, cycles: int) -> None:
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())
    dut.mrst.value = 1
    await RisingEdge(dut.clk)
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    dut.mrst.value = 0
    for _ in range(cycles):
        await RisingEdge(dut.clk)
    await Timer(1, unit="ns")


@cocotb.test()
async def check_observe(dut):
    # Every node kind is readable through the manifest, port-free.
    k = KSim(dut)
    await _reset_and_run(dut, 12)         # let the whole seq finish

    assert k.x.value       == 0xA5, f"internal reg: {k.x.value!s}"
    assert k.w.value       == 0xA6, f"comb wire x+1: {k.w.value!s}"
    assert k.hi.value      == 0xA,  f"sliced view [7:4]: {k.hi.value}"
    assert k.cnt.value     == 5,    f"counter committed reg: {k.cnt.value!s}"
    assert k.rf[0].data.value == 3, f"karray element: {k.rf[0].data.value!s}"
    assert k.sub.acc.value == 7,    f"sub-module internal reg: {k.sub.acc.value!s}"

    # A sliced view has no net of its own — writing it must refuse loudly.
    try:
        k.hi.value = 1
        raise AssertionError("slice write should have raised")
    except AttributeError:
        pass


@cocotb.test()
async def check_force(dut):
    # Deposits land on internal signals and downstream logic sees them.
    k = KSim(dut)
    await _reset_and_run(dut, 12)         # seq done — nothing overwrites x anymore

    k.x.value = 0x30                      # force the internal reg...
    await Timer(1, unit="ns")
    assert k.x.value == 0x30, f"forced reg did not hold: {k.x.value!s}"
    assert k.w.value == 0x31, f"comb logic missed the forced value: {k.w.value!s}"
    assert k.hi.value == 0x3, f"slice view of forced value: {k.hi.value}"

    k.rf[1].data.value = 9                # ...and a flow-unwritten karray element
    await Timer(1, unit="ns")
    assert k.rf[1].data.value == 9, f"forced karray element: {k.rf[1].data.value!s}"
    assert k.w2.value == 9, f"comb logic missed the forced element: {k.w2.value!s}"


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
