# tc28 — mem_blk + mem_ele: first coverage of the memory primitives.
#   * gated clocked write  : `with zif(wen == 1): wport |= wdata` (bare, outside seq)
#   * combinational read   : `rdata *= rport` — data visible the same cycle
#   * hierarchical poke    : the testbench preloads the memory array directly
#     (k.spad via KSim) before releasing mrst, then reads it back through the
#     read port — the program-loading pattern downstream projects rely on.

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog
from kathryn.sim_assist import KSim

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc28_mem_blk"


# ---- model -------------------------------------------------------------------
class tc28_mem_blk(Module):
    @init
    def com_declare(self):
        self.spad  = mem_blk(8, 4, "spad")
        self.waddr = wire(4, "waddr")
        self.wdata = wire(8, "wdata")
        self.wen   = wire(1, "wen")
        self.raddr = wire(4, "raddr")
        self.rdata = wire(8, "rdata")

        self.wport = mem_ele(self.spad, self.waddr, 8, False, "wport")
        self.rport = mem_ele(self.spad, self.raddr, 8, True,  "rport")

        self.waddr.mark_input ("waddr_in")
        self.wdata.mark_input ("wdata_in")
        self.wen.mark_input   ("wen_in")
        self.raddr.mark_input ("raddr_in")

    @flow
    def my_flow(self):
        with zif(self.wen == 1):
            self.wport |= self.wdata      # clocked write, gated by wen
        self.rdata *= self.rport          # combinational read


# ---- build -------------------------------------------------------------------
def build(output_folder: str) -> None:
    reset()
    build_model(tc28_mem_blk())
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
@cocotb.test()
async def check_write_then_read(dut):
    # Port-driven write, then combinational read-back of every address.
    k = KSim(dut)
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value     = 1
    dut.wen_in.value   = 0
    dut.waddr_in.value = 0
    dut.wdata_in.value = 0
    dut.raddr_in.value = 0
    await RisingEdge(dut.clk)
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    dut.mrst.value = 0

    data = {i: (0x10 + i * 3) & 0xFF for i in range(16)}

    # write one address per cycle through the write port
    for a, d in data.items():
        dut.wen_in.value   = 1
        dut.waddr_in.value = a
        dut.wdata_in.value = d
        await RisingEdge(dut.clk)
    dut.wen_in.value = 0
    await RisingEdge(dut.clk)

    # combinational read: set raddr, expect rdata after a settle delta
    for a, d in data.items():
        dut.raddr_in.value = a
        await Timer(1, unit="ns")
        got = int(k.rdata.value)
        assert got == d, f"mem[{a}] = {got:#x} (expected {d:#x})"


@cocotb.test()
async def check_hierarchical_poke(dut):
    # Preload the memory array from the testbench (no ports), then read back
    # through the read port — validates the program-loading pattern.
    k = KSim(dut)
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    mem = k.spad
    image = {i: (0xA0 ^ (i * 7)) & 0xFF for i in range(16)}
    for a, d in image.items():
        mem[a].value = d

    dut.mrst.value     = 1
    dut.wen_in.value   = 0
    dut.raddr_in.value = 0
    await RisingEdge(dut.clk)
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    dut.mrst.value = 0

    for a, d in image.items():
        dut.raddr_in.value = a
        await Timer(1, unit="ns")
        got = int(k.rdata.value)
        assert got == d, f"poked mem[{a}] = {got:#x} (expected {d:#x})"


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
