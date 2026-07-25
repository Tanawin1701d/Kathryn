# tc41 — signed arithmetic surface: .sra / .sdiv / .srem / .sle / .sge on
# SignalRef plus lib.arith.mulh in all three signedness combinations.
# All comb: drive the inputs, check every derived output the same cycle.

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog
from kathryn.lib import mulh

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc41_signed_ops"


# ---- model -------------------------------------------------------------------
class tc41_signed_ops(Module):
    @init
    def com_declare(self):
        self.a = wire(8, "a")
        self.b = wire(8, "b")
        self.n = wire(3, "n")                     # shift amount

        self.o_sra   = wire(8, "o_sra")
        self.o_sdiv  = wire(8, "o_sdiv")
        self.o_srem  = wire(8, "o_srem")
        self.o_sle   = wire(1, "o_sle")
        self.o_sge   = wire(1, "o_sge")
        self.o_mulhss = wire(8, "o_mulhss")       # signed   x signed
        self.o_mulhuu = wire(8, "o_mulhuu")       # unsigned x unsigned
        self.o_mulhsu = wire(8, "o_mulhsu")       # signed   x unsigned

        self.a.mark_input("a_in")
        self.b.mark_input("b_in")
        self.n.mark_input("n_in")
        for w, port in ((self.o_sra,    "sra_out"),    (self.o_sdiv,   "sdiv_out"),
                        (self.o_srem,   "srem_out"),   (self.o_sle,    "sle_out"),
                        (self.o_sge,    "sge_out"),    (self.o_mulhss, "mulhss_out"),
                        (self.o_mulhuu, "mulhuu_out"), (self.o_mulhsu, "mulhsu_out")):
            w.mark_output(port)

    @flow
    def my_flow(self):
        self.o_sra    *= self.a.sra(self.n)
        self.o_sdiv   *= self.a.sdiv(self.b)
        self.o_srem   *= self.a.srem(self.b)
        self.o_sle    *= self.a.sle(self.b)
        self.o_sge    *= self.a.sge(self.b)
        self.o_mulhss *= mulh(self.a, self.b, True,  True)
        self.o_mulhuu *= mulh(self.a, self.b, False, False)
        self.o_mulhsu *= mulh(self.a, self.b, True,  False)


# ---- build -------------------------------------------------------------------
def build(output_folder: str) -> None:
    reset()
    build_model(tc41_signed_ops())
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
def _s8(x: int) -> int:
    return x - 256 if x & 0x80 else x


def _trunc_div(a: int, b: int) -> int:
    # Verilog/RISC-V signed division truncates toward zero (Python // floors).
    q = abs(a) // abs(b)
    return -q if (a < 0) != (b < 0) else q


def _model(a, b, n):
    sa, sb = _s8(a), _s8(b)
    return {
        "sra_out"   : (sa >> n) & 0xFF,           # python >> on ints is arithmetic
        "sdiv_out"  : _trunc_div(sa, sb) & 0xFF,
        "srem_out"  : (sa - sb * _trunc_div(sa, sb)) & 0xFF,
        "sle_out"   : 1 if sa <= sb else 0,
        "sge_out"   : 1 if sa >= sb else 0,
        "mulhss_out": ((sa * sb) >> 8) & 0xFF,
        "mulhuu_out": ((a  * b ) >> 8) & 0xFF,
        "mulhsu_out": ((sa * b ) >> 8) & 0xFF,
    }


@cocotb.test()
async def check_signed_ops(dut):
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value = 1
    dut.a_in.value = 0
    dut.b_in.value = 1
    dut.n_in.value = 0
    await RisingEdge(dut.clk)
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    dut.mrst.value = 0

    vectors = [                                    # (a, b, n) — b never 0 (div)
        (0x00, 0x01, 0), (0xFF, 0x01, 1), (0x80, 0x7F, 7),
        (0x7F, 0xFF, 3), (0xA5, 0x03, 4), (0x5A, 0xFD, 2),
        (0x01, 0x80, 5), (0xC0, 0x40, 6), (0xFE, 0xFE, 1),
    ]
    for a, b, n in vectors:
        dut.a_in.value = a
        dut.b_in.value = b
        dut.n_in.value = n
        await Timer(2, unit="ns")
        for port, want in _model(a, b, n).items():
            got = int(getattr(dut, port).value)
            assert got == want, \
                f"a={a:#x} b={b:#x} n={n}: {port} = {got:#x} (expected {want:#x})"


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
