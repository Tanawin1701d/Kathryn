# tc10 — zero-cycle switch: out wire driven combinationally based on sel register.
# Three cases (0→10, 1→20, 2→30); testbench drives sel then reads out combinationally.

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc10_zswitch"


# ---- model -------------------------------------------------------------------
class tc10_zswitch(Module):
    @init
    def com_declare(self):
        self.sel   = reg(2, "sel")
        self.out   = wire(8, "out")
        self.val_a = val(8, 10, "val_a")
        self.val_b = val(8, 20, "val_b")
        self.val_c = val(8, 30, "val_c")

        self.sel.mark_output("my_sel")
        self.out.mark_output("my_out")

    @flow
    def my_flow(self):
        with seq():
            with zstate(self.sel):
                with zcase(0):
                    self.out *= self.val_a
                with zcase(1):
                    self.out *= self.val_b
                with zcase(2):
                    self.out *= self.val_c


# ---- build -------------------------------------------------------------------
def build(output_folder: str) -> None:
    reset()
    module = tc10_zswitch()
    build_model(module)
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
@cocotb.test()
async def check_zswitch(dut):
    # zstate/zcase is combinational: out changes the same cycle sel changes.
    # We write sel via direct force (it's an output reg, not an input), then
    # verify out reflects the matching val after a delta.
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value = 1
    await RisingEdge(dut.clk)            # E1
    await RisingEdge(dut.clk)            # E2
    await Timer(1, unit="ns")
    dut.mrst.value = 0

    # Let the sequence reach the zstate block.
    for _ in range(4):
        await RisingEdge(dut.clk)
    await Timer(1, unit="ns")

    # sel defaults to 0 (post-reset X, but zcase(0) arm active when sel==0).
    # Force sel to each value and check out combinationally.
    dut.my_sel.value = 0
    await Timer(1, unit="ns")
    assert dut.my_out.value == 10, f"sel=0: my_out = {dut.my_out.value!s} (expected 10)"

    dut.my_sel.value = 1
    await Timer(1, unit="ns")
    assert dut.my_out.value == 20, f"sel=1: my_out = {dut.my_out.value!s} (expected 20)"

    dut.my_sel.value = 2
    await Timer(1, unit="ns")
    assert dut.my_out.value == 30, f"sel=2: my_out = {dut.my_out.value!s} (expected 30)"


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
