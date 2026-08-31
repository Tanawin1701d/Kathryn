# tc18 — int literals as operands / assignment sources are auto-wrapped into a
# width-matched val (no manual val(...) needed). Exhaustively exercises EVERY
# overloaded operator, end to end in simulation:
#
#   forward (signal ∘ int):  + - * / %   & | ^ << >>   ~(unary)   == != < <= > >=
#   reflected (int ∘ signal): __radd/rsub/rmul/rtruediv/rmod/rand/ror/rxor/rlshift/rrshift
#   auto-reflected compares:  int < signal, int > signal, int == signal
#                             (Python flips these to a.__gt__/__lt__/__eq__)
#
# `a` (=10) and `b` (=2, used as a small shift amount) are reset-pinned, so every
# result reg latches a deterministic constant during the sequence and holds it.

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog
from kathryn.sim_assist import KSim

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc18_int_operand_autowrap"

A = 10        # a is reset-pinned here (8-bit)
B = 2         # b is reset-pinned here (8-bit) — small shift amount for reflected shifts

# Expected outputs computed with the same (masked) semantics the 8-bit RTL uses.
M = 0xFF
EXPECT = {
    # ---- forward arithmetic (signal ∘ int) ----
    "my_add":  (A + 5)  & M,        # 15
    "my_sub":  (A - 3)  & M,        # 7
    "my_mul":  (A * 3)  & M,        # 30
    "my_div":  A // 3,              # 3
    "my_mod":  A % 3,               # 1
    # ---- forward bitwise (signal ∘ int) + unary invert ----
    "my_and":  A & 0x0C,            # 8
    "my_or":   A | 0x01,            # 11
    "my_xor":  A ^ 0x0F,            # 5
    "my_shl":  (A << 2) & M,        # 40
    "my_shr":  A >> 1,              # 5
    "my_inv":  (~A) & M,            # 245
    # ---- reflected arithmetic/bitwise (int ∘ signal) ----
    "my_radd": (5 + A)   & M,       # 15
    "my_rsub": (100 - A) & M,       # 90  (order kept: 100 - a, not a - 100)
    "my_rmul": (3 * A)   & M,       # 30
    "my_rdiv": 100 // A,            # 10
    "my_rmod": 105 % A,             # 5
    "my_rand": 0x0C & A,            # 8
    "my_ror":  0x01 | A,            # 11
    "my_rxor": 0x0F ^ A,            # 5
    "my_rlsh": (1 << B)   & M,      # 4   (1 << b)
    "my_rrsh": 240 >> B,            # 60  (240 >> b)
    # ---- forward relational (signal ∘ int) -> 1 bit, mixed true/false ----
    "my_eq":   int(A == 10),        # 1
    "my_ne":   int(A != 5),         # 1
    "my_lt":   int(A < 5),          # 0
    "my_le":   int(A <= 9),         # 0
    "my_gt":   int(A > 5),          # 1
    "my_ge":   int(A >= 11),        # 0
    # ---- auto-reflected compares (int ∘ signal) ----
    "my_alt":  int(20 > A),         # 1  -> a.__lt__(20)
    "my_agt":  int(5 < A),          # 1  -> a.__gt__(5)
    "my_aeq":  int(10 == A),        # 1  -> a.__eq__(10)
}


# ---- model -------------------------------------------------------------------
class tc18_int_operand_autowrap(Module):
    @init
    def com_declare(self):
        self.a = reg(8, "a"); self.a.reset(A)      # pinned source
        self.b = reg(8, "b"); self.b.reset(B)      # pinned shift amount

        # One result reg per case (8-bit, except the 1-bit relational results).
        # A non-underscore dict attribute becomes a "dict" manifest node, so the
        # sim reads every reg through KSim (`k.out[name]`) — no ports needed.
        self.out = {}
        for name in EXPECT:
            width = 1 if name in ("my_eq", "my_ne", "my_lt", "my_le", "my_gt",
                                  "my_ge", "my_alt", "my_agt", "my_aeq") else 8
            self.out[name] = reg(width, name + "_r")

    @flow
    def my_flow(self):
        a, b, o = self.a, self.b, self.out
        with seq():
            # forward arithmetic
            o["my_add"] |= a + 5
            o["my_sub"] |= a - 3
            o["my_mul"] |= a * 3
            o["my_div"] |= a / 3
            o["my_mod"] |= a % 3
            # forward bitwise + unary
            o["my_and"] |= a & 0x0C
            o["my_or"]  |= a | 0x01
            o["my_xor"] |= a ^ 0x0F
            o["my_shl"] |= a << 2
            o["my_shr"] |= a >> 1
            o["my_inv"] |= ~a
            # reflected arithmetic/bitwise
            o["my_radd"] |= 5 + a
            o["my_rsub"] |= 100 - a
            o["my_rmul"] |= 3 * a
            o["my_rdiv"] |= 100 / a
            o["my_rmod"] |= 105 % a
            o["my_rand"] |= 0x0C & a
            o["my_ror"]  |= 0x01 | a
            o["my_rxor"] |= 0x0F ^ a
            o["my_rlsh"] |= 1 << b
            o["my_rrsh"] |= 240 >> b
            # forward relational
            o["my_eq"] |= a == 10
            o["my_ne"] |= a != 5
            o["my_lt"] |= a < 5
            o["my_le"] |= a <= 9
            o["my_gt"] |= a > 5
            o["my_ge"] |= a >= 11
            # auto-reflected compares
            o["my_alt"] |= 20 > a
            o["my_agt"] |= 5 < a
            o["my_aeq"] |= 10 == a


# ---- build (kathryn model -> verilog) ---------------------------------------
def build(output_folder: str) -> None:
    reset()
    module = tc18_int_operand_autowrap()
    build_model(module)
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
@cocotb.test()
async def check_int_operand_autowrap(dut):
    # 10ns clock; assert master-reset for two cycles to launch the sequence.
    k = KSim(dut)
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())

    dut.mrst.value = 1
    await RisingEdge(dut.clk)            # E1: start <= 1
    await RisingEdge(dut.clk)            # E2: start held
    await Timer(1, unit="ns")
    dut.mrst.value = 0

    # Run well past the ~30-step sequence so every result reg has latched and holds.
    for _ in range(60):
        await RisingEdge(dut.clk)
    await Timer(1, unit="ns")

    for name, want in EXPECT.items():
        got = int(k.out[name].value)
        assert got == want, f"{name} = {got} (expected {want})"


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
