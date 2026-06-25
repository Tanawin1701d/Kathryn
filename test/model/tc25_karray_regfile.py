# tc25 — Karray (typed multi-dimensional array CCP) as a tiny register file.
#
# `rf` is a 4-entry Karray whose element is a {valid:1, data:7} record — each field
# is its OWN reg (per-field HCP, not a packed bit-slice). The top `seq` runs ONCE
# (one-shot state machine), so this is a deterministic test: write known constants,
# read them back through plain output regs, and check exact values. Two write styles
# are exercised:
#   * entry 0 — FIELD-WISE writes: rf[0].valid <= 1, rf[0].data <= 42
#   * entry 1 — WHOLE-ELEMENT write: rf[1] <= {"valid": 1, "data": 42}, each named
#               source connected to the field of that name (no bit-level split)
#
# Intended behaviour (what this testbench asserts):
#   * field-wise path: my_v == 1 and my_d == 42 once the sequence has run.
#   * whole-element named write: my_pv == 1 and my_pd == 42, proving each named
#     source lands on its matching field.
#   * under held master reset every OUTPUT reg stays at its reset value 0.

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc25_karray_regfile"

SETTLE_CYCLES = 24                      # cycles for the one-shot sequence to complete
DATA          = 42                      # 0x2A — the value written into both data fields


# ---- model -------------------------------------------------------------------
class tc25_karray_regfile(Module):
    @init
    def com_declare(self):
        # 4-entry register file; each entry a {valid:1, data:7} record, each field
        # materialised as its own reg (reg backing -> clocked, assigned with |=).
        self.rf = Karray((4,), [("valid", 1), ("data", 7)], HwComponentType.REG, "rf")

        # constants written into the file
        self.c_valid = val(1, 1,    "c_valid")
        self.c_data  = val(7, DATA, "c_data")
        self.c_pvalid = val(1, 1,    "c_pvalid")  # whole-element named sources (per field)
        self.c_pdata  = val(7, DATA, "c_pdata")

        # outputs that mirror the read-back fields (so cocotb can observe them)
        self.o_valid  = reg(1, "o_valid")        # <- rf[0].valid (field-wise)
        self.o_data   = reg(7, "o_data")         # <- rf[0].data  (field-wise)
        self.o_pvalid = reg(1, "o_pvalid")       # <- rf[1].valid (whole-element split)
        self.o_pdata  = reg(7, "o_pdata")        # <- rf[1].data  (whole-element split)

        self.o_valid.mark_output("my_v")
        self.o_data.mark_output("my_d")
        self.o_pvalid.mark_output("my_pv")
        self.o_pdata.mark_output("my_pd")

    @flow
    def my_flow(self):
        self.o_valid.reset(0)
        self.o_data.reset(0)
        self.o_pvalid.reset(0)
        self.o_pdata.reset(0)

        with seq():
            # entry 0 — field-wise writes into the per-field regs
            self.rf[0].valid |= self.c_valid
            self.rf[0].data  |= self.c_data

            # entry 1 — whole-element write; each named source lands on its field
            self.rf[1] |= {"valid": self.c_pvalid, "data": self.c_pdata}

            # read both entries back out through the output regs
            self.o_valid  |= self.rf[0].valid
            self.o_data   |= self.rf[0].data
            self.o_pvalid |= self.rf[1].valid
            self.o_pdata  |= self.rf[1].data


# ---- build (kathryn model -> verilog) ---------------------------------------
def build(output_folder: str) -> None:
    reset()
    module = tc25_karray_regfile()
    build_model(module)
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
async def _reset_and_release(dut):
    # Drive the clock, hold master reset for two edges (regs load 0, start arms),
    # then release it. Returns once reset is deasserted just after an edge.
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())
    dut.mrst.value = 1
    await RisingEdge(dut.clk)            # E1
    await RisingEdge(dut.clk)            # E2
    await Timer(1, unit="ns")
    dut.mrst.value = 0


async def _settle(dut):
    # Run the one-shot sequence to completion, then return the final outputs.
    await _reset_and_release(dut)
    for _ in range(SETTLE_CYCLES):
        await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    return (int(dut.my_v.value), int(dut.my_d.value),
            int(dut.my_pv.value), int(dut.my_pd.value))


@cocotb.test()
async def check_field_write_read(dut):
    # Field-wise writes land in the per-field regs and read back exactly.
    v, d, _, _ = await _settle(dut)
    assert v == 1,    f"valid field wrong: my_v={v} (want 1)"
    assert d == DATA, f"data field wrong: my_d={d} (want {DATA})"


@cocotb.test()
async def check_whole_element_named(dut):
    # The whole-element write rf[1] <= {"valid": 1, "data": 42} connects each named
    # source to its matching field: valid <= 1, data <= 42.
    _, _, pv, pd = await _settle(dut)
    assert pv == 1,    f"named valid wrong: my_pv={pv} (want 1)"
    assert pd == DATA, f"named data wrong: my_pd={pd} (want {DATA})"


@cocotb.test()
async def check_reset_clears(dut):
    # While master reset is held every output reg is pinned to its reset value 0.
    await _reset_and_release(dut)
    dut.mrst.value = 1                   # re-assert and keep it asserted
    for _ in range(4):
        await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    vals = (int(dut.my_v.value), int(dut.my_d.value),
            int(dut.my_pv.value), int(dut.my_pd.value))
    assert vals == (0, 0, 0, 0), f"reset did not clear the outputs: {vals}"


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
