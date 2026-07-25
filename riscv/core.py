# RV64 multi-cycle core. One Kathryn Module: hardware in @init, everything else
# split into two @flow sections — pure combinational decode/ALU logic first
# (always-active, zero-cycle), then the fetch/execute FSM (seq + cwhile, the
# tc38 processor-loop pattern). All encodings come from isa.py; all sizing and
# feature gating from CoreConfig (plain elaboration-time Python).
#
# Bus protocol (see tools/simharness/README.md): hold req+payload until ack is
# sampled high at a posedge, then deassert req for >= 1 cycle.

from __future__ import annotations

from kathryn import (Module, init, flow, reg, wire, val, seq, par, cwhile, scwait, sywait,
                     cif, cselif, cselse, zif, zelif, zelse,
                     Karray, kaf, HwComponentType)
from kathryn.lib import mux, muxn, or_reduce, sext

from . import isa
from .config import CoreConfig


class RegSet(Karray):
    x = kaf(64)                            # one 64-bit field per architectural register


class RV64Core(Module):
    def __init__(self, cfg: CoreConfig, name=None):
        self.cfg = cfg
        super().__init__(name)

    # ---- hardware -----------------------------------------------------------
    @init
    def com_declare(self):
        cfg  = self.cfg
        XLEN = cfg.xlen

        # architectural state
        self.pc     = reg(XLEN, "pc")
        self.rf     = RegSet(HwComponentType.REG, (32,), "rf")
        self.instr  = reg(32, "instr")
        self.ld_raw = reg(XLEN, "ld_raw")          # raw load data latched off the bus
        self.halted = reg(1, "halted")             # set on unknown opcode (debug aid)

        # bus master (registered request, protocol in the header comment)
        self.b_req   = reg(1,    "b_req")
        self.b_we    = reg(1,    "b_we")
        self.b_addr  = reg(XLEN, "b_addr")
        self.b_wdata = reg(XLEN, "b_wdata")
        self.b_size  = reg(2,    "b_size")
        self.b_ack   = wire(1,    "b_ack")
        self.b_rdata = wire(XLEN, "b_rdata")

        # interrupt pins (consumed from M2 on)
        self.irq_mtip = wire(1, "irq_mtip")
        self.irq_msip = wire(1, "irq_msip")
        self.irq_meip = wire(1, "irq_meip")

        # decode fields (dedicated wires: karray/muxn indices reject slice views)
        self.dec_op  = wire(7, "dec_op")
        self.dec_rd  = wire(5, "dec_rd")
        self.dec_f3  = wire(3, "dec_f3")
        self.dec_rs1 = wire(5, "dec_rs1")
        self.dec_rs2 = wire(5, "dec_rs2")

        # datapath wires
        self.rs1_val    = wire(XLEN, "rs1_val")
        self.rs2_val    = wire(XLEN, "rs2_val")
        self.wb_val     = wire(XLEN, "wb_val")
        self.wb_en      = wire(1,    "wb_en")
        self.next_pc    = wire(XLEN, "next_pc")
        self.mem_addr_c = wire(XLEN, "mem_addr_c") # load/store effective address
        self.mem_size_c = wire(2,    "mem_size_c")
        self.load_val   = wire(XLEN, "load_val")
        self.is_load    = wire(1,    "is_load")
        self.is_store   = wire(1,    "is_store")
        self.halt_req   = wire(1,    "halt_req")

        # IO: the external-bus contract
        self.b_req  .mark_output("mem_req")
        self.b_we   .mark_output("mem_we")
        self.b_addr .mark_output("mem_addr")
        self.b_wdata.mark_output("mem_wdata")
        self.b_size .mark_output("mem_size")
        self.b_ack  .mark_input ("mem_ack")
        self.b_rdata.mark_input ("mem_rdata")
        self.irq_mtip.mark_input("mtip")
        self.irq_msip.mark_input("msip")
        self.irq_meip.mark_input("meip")

    # ---- combinational decode + datapath (always active) --------------------
    @flow
    def comb_logic(self):
        cfg  = self.cfg
        XLEN = cfg.xlen
        ins  = self.instr

        # resets
        self.pc.reset(cfg.reset_vector)
        self.instr.reset(0)
        self.halted.reset(0)
        self.b_req.reset(0)

        # instruction fields
        self.dec_op  *= isa.opcode(ins)
        self.dec_rd  *= isa.rd(ins)
        self.dec_f3  *= isa.funct3(ins)
        self.dec_rs1 *= isa.rs1(ins)
        self.dec_rs2 *= isa.rs2(ins)

        # immediates
        imm_i = isa.imm_i(ins, XLEN)
        imm_s = isa.imm_s(ins, XLEN)
        imm_b = isa.imm_b(ins, XLEN)
        imm_u = isa.imm_u(ins, XLEN)
        imm_j = isa.imm_j(ins, XLEN)

        # register-file read (x0 reads as zero; x0 is never written)
        self.rs1_val *= mux(self.dec_rs1 == 0, 0, self.rf[self.dec_rs1].x, "rs1_mux")
        self.rs2_val *= mux(self.dec_rs2 == 0, 0, self.rf[self.dec_rs2].x, "rs2_mux")

        # instruction-class flags
        self.is_load  *= self.dec_op == isa.OP_LOAD
        self.is_store *= self.dec_op == isa.OP_STORE
        is_reg_op      = (self.dec_op == isa.OP_OP).lor(self.dec_op == isa.OP_OP32)

        # ---- ALU ----
        op_a  = self.rs1_val
        op_b  = wire(XLEN, "op_b")
        op_b *= mux(is_reg_op, self.rs2_val, imm_i, "op_b_mux")

        sr_arith = ins[30]                             # SRA/SRAI vs SRL/SRLI
        is_sub   = (self.dec_op == isa.OP_OP).land(ins[30])
        is_sub_w = (self.dec_op == isa.OP_OP32).land(ins[30])
        shamt6   = wire(6, "shamt6"); shamt6 *= op_b[5, 0]
        shamt5   = wire(5, "shamt5"); shamt5 *= op_b[4, 0]

        alu_out  = wire(XLEN, "alu_out")
        alu_out *= muxn(self.dec_f3, [
            mux(is_sub, op_a - op_b, op_a + op_b, "addsub"),        # 0 ADD/SUB
            op_a << shamt6,                                          # 1 SLL
            op_a.slt(op_b).extend(XLEN),                             # 2 SLT
            (op_a < op_b).extend(XLEN),                              # 3 SLTU
            op_a ^ op_b,                                             # 4 XOR
            mux(sr_arith, op_a.sra(shamt6), op_a >> shamt6, "sr"),   # 5 SRL/SRA
            op_a | op_b,                                             # 6 OR
            op_a & op_b,                                             # 7 AND
        ], "alu_mux")

        # 32-bit W-ops: compute on the low word, sign-extend the 32-bit result
        a32, b32 = op_a[31, 0], op_b[31, 0]
        w_res    = wire(32, "w_res")
        w_res   *= muxn(self.dec_f3, [
            mux(is_sub_w, a32 - b32, a32 + b32, "addsub_w"),         # 0 ADDW/SUBW
            a32 << shamt5,                                           # 1 SLLW
            mux(sr_arith, a32.sra(shamt5), a32 >> shamt5, "sr_w"),   # 5 SRLW/SRAW (others unused)
        ], "w_mux")
        alu_w64 = sext(w_res, XLEN, "alu_w64")

        # ---- branch condition ----
        br_eq  = self.rs1_val == self.rs2_val
        br_lt  = self.rs1_val.slt(self.rs2_val)
        br_ltu = self.rs1_val < self.rs2_val
        br_taken = wire(1, "br_taken")
        br_taken *= muxn(self.dec_f3, [
            br_eq, br_eq.lnot(), 0, 0,                               # BEQ/BNE (2,3 unused)
            br_lt, br_lt.lnot(), br_ltu, br_ltu.lnot(),              # BLT/BGE/BLTU/BGEU
        ], "br_mux")

        # ---- write-back value / enable ----
        pc_plus4 = self.pc + 4
        with zif(self.dec_op == isa.OP_LUI):
            self.wb_val *= imm_u
        with zelif(self.dec_op == isa.OP_AUIPC):
            self.wb_val *= self.pc + imm_u
        with zelif(self.dec_op == isa.OP_JAL):
            self.wb_val *= pc_plus4
        with zelif(self.dec_op == isa.OP_JALR):
            self.wb_val *= pc_plus4
        with zelif(self.dec_op == isa.OP_LOAD):
            self.wb_val *= self.load_val
        with zelif(self.dec_op == isa.OP_IMM32):
            self.wb_val *= alu_w64
        with zelif(self.dec_op == isa.OP_OP32):
            self.wb_val *= alu_w64
        with zelse():
            self.wb_val *= alu_out                                    # OP / OP_IMM

        self.wb_en   *= or_reduce([self.dec_op == op for op in isa.WB_OPCODES])
        self.halt_req *= or_reduce([self.dec_op == op for op in isa.known_opcodes(cfg)]).lnot()

        # ---- next pc ----
        with zif(self.dec_op == isa.OP_JAL):
            self.next_pc *= self.pc + imm_j
        with zelif(self.dec_op == isa.OP_JALR):
            self.next_pc *= (self.rs1_val + imm_i) & 0xFFFF_FFFF_FFFF_FFFE
        with zelif((self.dec_op == isa.OP_BRANCH).land(br_taken)):
            self.next_pc *= self.pc + imm_b
        with zelse():
            self.next_pc *= pc_plus4

        # ---- load/store effective address + size ----
        self.mem_addr_c *= self.rs1_val + mux(self.is_store, imm_s, imm_i, "ea_imm")
        self.mem_size_c *= ins[13, 12]

        # ---- load-data extension (harness returns addressed bytes in low lanes) ----
        ld = self.ld_raw
        self.load_val *= muxn(self.dec_f3, [
            sext(ld[7, 0],  XLEN, "ld_b"),                           # 0 LB
            sext(ld[15, 0], XLEN, "ld_h"),                           # 1 LH
            sext(ld[31, 0], XLEN, "ld_w"),                           # 2 LW
            ld,                                                      # 3 LD
            ld[7, 0].extend(XLEN),                                   # 4 LBU
            ld[15, 0].extend(XLEN),                                  # 5 LHU
            ld[31, 0].extend(XLEN),                                  # 6 LWU
            ld,                                                      # 7 (unused)
        ], "ld_mux")

    # ---- the fetch/execute FSM ----------------------------------------------
    @flow
    def fsm(self):
        with seq():
            with cwhile(self.halted == 0):
                # FETCH: issue a 4-byte read at pc
                with par():
                    self.b_req  |= 1
                    self.b_we   |= 0
                    self.b_addr |= self.pc
                    self.b_size |= 2
                scwait(self.b_ack)
                with par():
                    self.instr |= self.b_rdata[31, 0]
                    self.b_req |= 0
                # decode settle: the cif chain below samples its conditions at the
                # posedge that latched `instr`, so give the comb decode one cycle
                sywait(1)

                # MEM: loads / stores make one extra bus access
                with cif(self.is_load == 1):
                    with par():
                        self.b_req  |= 1
                        self.b_we   |= 0
                        self.b_addr |= self.mem_addr_c
                        self.b_size |= self.mem_size_c
                    scwait(self.b_ack)
                    with par():
                        self.ld_raw |= self.b_rdata
                        self.b_req  |= 0
                with cselif(self.is_store == 1):
                    with par():
                        self.b_req   |= 1
                        self.b_we    |= 1
                        self.b_addr  |= self.mem_addr_c
                        self.b_wdata |= self.rs2_val
                        self.b_size  |= self.mem_size_c
                    scwait(self.b_ack)
                    self.b_req |= 0

                # WB: register write, pc advance, halt latch — one cycle
                with par():
                    self.rf.cus_dynamic_assign(
                        [slice(1, 32)], {"x": self.wb_val},           # x0 excluded
                        lambda v: self.wb_en & (self.dec_rd == v.coord[0]),
                    )
                    self.pc     |= self.next_pc
                    self.halted |= self.halted | self.halt_req
