# tc42 — mem_blk(init_file=...): $readmemh preload emitted by the backend.
# The build step writes a known hex image next to the Verilog; the sim then
# reads every address back through a combinational read port with NO testbench
# poking — contents must come from the initial $readmemh alone.

from __future__ import annotations

from pathlib import Path

from kathryn import *
from kathryn import emit_verilog

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc42_mem_init"

# Deterministic 16-entry image shared by build (hex writer) and sim (golden).
IMAGE = {i: (0xC3 ^ (i * 11)) & 0xFF for i in range(16)}


# ---- model -------------------------------------------------------------------
class tc42_mem_init(Module):
    def __init__(self, init_file: str, name=None):
        self._init_file = init_file
        super().__init__(name)

    @init
    def com_declare(self):
        self.rom   = mem_blk(8, 4, "rom", init_file=self._init_file)
        self.raddr = wire(4, "raddr")
        self.rdata = wire(8, "rdata")
        self.rport = mem_ele(self.rom, self.raddr, 8, True, "rport")

        self.raddr.mark_input ("raddr_in")
        self.rdata.mark_output("rdata_out")

    @flow
    def my_flow(self):
        self.rdata *= self.rport          # combinational read


# ---- build -------------------------------------------------------------------
def build(output_folder: str) -> None:
    hex_path = Path(output_folder) / "rom_image.hex"
    hex_path.write_text("\n".join(f"{IMAGE[i]:02x}" for i in range(16)) + "\n")
    reset()
    build_model(tc42_mem_init(str(hex_path)))
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
@cocotb.test()
async def check_readmemh(dut):
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value     = 1
    dut.raddr_in.value = 0
    await RisingEdge(dut.clk)
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    dut.mrst.value = 0

    for a, d in IMAGE.items():
        dut.raddr_in.value = a
        await Timer(1, unit="ns")
        got = int(dut.rdata_out.value)
        assert got == d, f"rom[{a}] = {got:#x} (expected {d:#x} from $readmemh)"


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
