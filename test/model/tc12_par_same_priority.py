# tc12 — parallel auto-sync, three branches assigning the SAME register at the
# SAME priority. Priorities are written relative to the default user priority so
# the relationship is explicit: all three sit at DEFAULT_UE_PRI_USER + 1. When the
# priority (and sub-priority) tie, the update pool's stable sort keeps program
# order, so the LAST assignment is emitted last and wins under non-blocking
# semantics. Here x<=5, x<=10, x<=15 all @(+1) → x settles to 15 (the last one).

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc12_par_same_priority"

# All branches share one priority — pick a single offset above the user default.
SAME_PRI = DEFAULT_UE_PRI_USER + 1


# ---- model -------------------------------------------------------------------
class tc12_par_same_priority(Module):
    @init
    def com_declare(self):
        self.x      = reg(8, "x")
        self.val_5  = val(8, 5,  "val_5")
        self.val_10 = val(8, 10, "val_10")
        self.val_15 = val(8, 15, "val_15")

        self.x.mark_output("my_x")

    @flow
    def my_flow(self):
        with seq():
            with par_auto():
                # Same destination, same priority → tie broken by program order.
                with priority(SAME_PRI):
                    self.x |= self.val_5        # x <= 5   (earliest)
                with priority(SAME_PRI):
                    self.x |= self.val_10       # x <= 10
                with priority(SAME_PRI):
                    self.x |= self.val_15       # x <= 15  (latest → wins on tie)


# ---- build -------------------------------------------------------------------
def build(output_folder: str) -> None:
    reset()
    module = tc12_par_same_priority()
    build_model(module)
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
@cocotb.test()
async def check_par_same_priority(dut):
    # 10ns clock; two cycles of master-reset.
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value = 1
    await RisingEdge(dut.clk)            # E1
    await RisingEdge(dut.clk)            # E2
    await Timer(1, unit="ns")
    dut.mrst.value = 0

    # E3: start -> par_auto body entered next cycle; x not latched yet.
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert dut.my_x.value != 15, f"my_x latched too early: {dut.my_x.value!s}"

    # E4: all three same-priority writes fire this cycle; the last one (x<=15) wins.
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert dut.my_x.value == 15, f"my_x = {dut.my_x.value!s} (expected 15 — last same-priority write wins)"

    # x holds 15 — nothing else drives it.
    for _ in range(10):
        await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert dut.my_x.value == 15, f"my_x drifted off 15: {dut.my_x.value!s}"


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
