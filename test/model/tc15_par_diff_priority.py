# tc15 — parallel auto-sync, three branches assigning the SAME register at THREE
# DIFFERENT priorities, written relative to the default user priority so the
# gradient is explicit: DEFAULT_UE_PRI_USER + 1 / + 2 / + 3. Priority overrides
# program order: the highest-priority write is sorted last in the update pool and
# wins under non-blocking semantics, even when it is declared FIRST.
#
# Here the highest priority (+3) is put on the first branch (x<=5), so program
# order alone would pick the last branch (x<=15) — but priority makes x<=5 win.
# x settles to 5, proving priority, not declaration order, decides.

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog
from kathryn.sim_assist import KSim

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc15_par_diff_priority"

# Three distinct priorities, each one step above the user default.
PRI_LOW  = DEFAULT_UE_PRI_USER + 1
PRI_MID  = DEFAULT_UE_PRI_USER + 2
PRI_HIGH = DEFAULT_UE_PRI_USER + 3


# ---- model -------------------------------------------------------------------
class tc15_par_diff_priority(Module):
    @init
    def com_declare(self):
        self.x      = reg(8, "x")
        self.val_5  = val(8, 5,  "val_5")
        self.val_10 = val(8, 10, "val_10")
        self.val_15 = val(8, 15, "val_15")

    @flow
    def my_flow(self):
        with seq():
            with par_auto():
                # Same destination, different priority → priority decides, not order.
                with priority(PRI_HIGH):
                    self.x |= self.val_5        # x <= 5   (declared FIRST, highest → wins)
                with priority(PRI_LOW):
                    self.x |= self.val_10       # x <= 10  (lowest priority)
                with priority(PRI_MID):
                    self.x |= self.val_15       # x <= 15  (declared LAST, but loses)


# ---- build -------------------------------------------------------------------
def build(output_folder: str) -> None:
    reset()
    module = tc15_par_diff_priority()
    build_model(module)
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
@cocotb.test()
async def check_par_diff_priority(dut):
    # 10ns clock; two cycles of master-reset.
    k = KSim(dut)
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value = 1
    await RisingEdge(dut.clk)            # E1
    await RisingEdge(dut.clk)            # E2
    await Timer(1, unit="ns")
    dut.mrst.value = 0

    # E3: start -> par_auto body entered next cycle; x not latched yet.
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert k.x.value != 5, f"my_x latched too early: {k.x.value!s}"

    # E4: all three writes fire this cycle; the highest-priority one (+3, x<=5)
    # wins even though x<=15 was declared last.
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert k.x.value == 5,  f"my_x = {k.x.value!s} (expected 5 — highest-priority write wins)"
    assert k.x.value != 15, f"my_x = {k.x.value!s} (last-declared must not win over higher priority)"

    # x holds 5 — nothing else drives it.
    for _ in range(10):
        await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert k.x.value == 5, f"my_x drifted off 5: {k.x.value!s}"


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
