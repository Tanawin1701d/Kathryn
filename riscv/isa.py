# RV64 instruction encodings and elaboration-time decode helpers. Pure data +
# functions that turn an `instr` signal into field/immediate expressions — no
# hardware state lives here. Every magic number in the core comes from this file.

from __future__ import annotations

from kathryn import val
from kathryn.lib import cat, sext

# ---- opcodes (instr[6:0]) ----------------------------------------------------
OP_LUI      = 0x37
OP_AUIPC    = 0x17
OP_JAL      = 0x6F
OP_JALR     = 0x67
OP_BRANCH   = 0x63
OP_LOAD     = 0x03
OP_STORE    = 0x23
OP_IMM      = 0x13
OP_OP       = 0x33
OP_IMM32    = 0x1B
OP_OP32     = 0x3B
OP_MISCMEM  = 0x0F
OP_SYSTEM   = 0x73
OP_AMO      = 0x2F

# ---- funct3 ------------------------------------------------------------------
F3_ADD_SUB, F3_SLL, F3_SLT, F3_SLTU, F3_XOR, F3_SRL_SRA, F3_OR, F3_AND = range(8)
F3_BEQ, F3_BNE, F3_BLT, F3_BGE, F3_BLTU, F3_BGEU = 0, 1, 4, 5, 6, 7

# ---- register-writing opcodes ------------------------------------------------
WB_OPCODES = (OP_LUI, OP_AUIPC, OP_JAL, OP_JALR, OP_LOAD, OP_IMM, OP_OP, OP_IMM32, OP_OP32)

# Opcodes the core implements (per extension set); anything else halts the core.
def known_opcodes(cfg) -> tuple:
    ops = [*WB_OPCODES, OP_BRANCH, OP_STORE, OP_MISCMEM]
    if cfg.ext_zicsr: ops.append(OP_SYSTEM)
    if cfg.ext_a:     ops.append(OP_AMO)
    return tuple(ops)


# ---- field extraction (slice views — no hardware created) --------------------
def opcode(instr): return instr[6, 0]
def rd    (instr): return instr[11, 7]
def funct3(instr): return instr[14, 12]
def rs1   (instr): return instr[19, 15]
def rs2   (instr): return instr[24, 20]
def funct7(instr): return instr[31, 25]


# ---- immediates (build sext/cat wires — call from @flow) ---------------------
def imm_i(instr, xlen):
    return sext(instr[31, 20], xlen, "imm_i")

def imm_s(instr, xlen):
    return sext(cat(instr[31, 25], instr[11, 7], name="imm_s_raw"), xlen, "imm_s")

def imm_b(instr, xlen):
    return sext(cat(instr[31], instr[7], instr[30, 25], instr[11, 8], val(1, 0),
                    name="imm_b_raw"), xlen, "imm_b")

def imm_u(instr, xlen):
    return sext(cat(instr[31, 12], val(12, 0), name="imm_u_raw"), xlen, "imm_u")

def imm_j(instr, xlen):
    return sext(cat(instr[31], instr[19, 12], instr[20], instr[30, 21], val(1, 0),
                    name="imm_j_raw"), xlen, "imm_j")
