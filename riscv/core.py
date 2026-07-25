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
from kathryn.lib import mux, muxn, mulh, or_reduce, sext

from . import isa
from .config import CoreConfig
from .csr import CsrFile


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
        self.op_unknown = wire(1,    "op_unknown")
        self.pc_redirect = wire(1,   "pc_redirect")   # taken jump/branch this instr

        if cfg.ext_a:
            self.resv      = reg(1,    "resv")        # LR reservation (single hart)
            self.resv_addr = reg(XLEN, "resv_addr")
            self.is_amo    = wire(1,    "is_amo")
            self.amo_is_lr = wire(1,    "amo_is_lr")
            self.amo_is_sc = wire(1,    "amo_is_sc")
            self.sc_fail   = wire(1,    "sc_fail")
            self.amo_result = wire(XLEN, "amo_result")

        if cfg.ext_zicsr:
            self.csr = CsrFile(cfg)
            self.csr.declare()

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

        # ---- M extension: mul/div (always reg-reg, funct7 == 1) ----
        if cfg.ext_m:
            dec_f7 = wire(7, "dec_f7"); dec_f7 *= ins[31, 25]
            self.is_md = wire(1, "is_md"); self.is_md *= dec_f7 == 1
            md_a, md_b = self.rs1_val, self.rs2_val

            zero_b  = md_b == 0
            negmax  = (md_a == 0x8000_0000_0000_0000).land(md_b == 0xFFFF_FFFF_FFFF_FFFF)
            md_out  = wire(XLEN, "md_out")
            md_out *= muxn(self.dec_f3, [
                md_a * md_b,                                                  # 0 MUL
                mulh(md_a, md_b, True,  True,  "mulh_ss"),                    # 1 MULH
                mulh(md_a, md_b, True,  False, "mulh_su"),                    # 2 MULHSU
                mulh(md_a, md_b, False, False, "mulh_uu"),                    # 3 MULHU
                mux(zero_b, 0xFFFF_FFFF_FFFF_FFFF,
                    mux(negmax, md_a, md_a.sdiv(md_b), "div_ovf"), "div_z"),  # 4 DIV
                mux(zero_b, 0xFFFF_FFFF_FFFF_FFFF, md_a / md_b, "divu_z"),    # 5 DIVU
                mux(zero_b, md_a, mux(negmax, 0, md_a.srem(md_b), "rem_ovf"), "rem_z"),  # 6 REM
                mux(zero_b, md_a, md_a % md_b, "remu_z"),                     # 7 REMU
            ], "md_mux")
            self.md_out = md_out

            # W variants on materialized 32-bit operand wires
            md_a32 = wire(32, "md_a32"); md_a32 *= md_a[31, 0]
            md_b32 = wire(32, "md_b32"); md_b32 *= md_b[31, 0]
            zero_bw = md_b32 == 0
            negmaxw = (md_a32 == 0x8000_0000).land(md_b32 == 0xFFFF_FFFF)
            mulw    = md_a32 * md_b32
            mdw     = wire(32, "mdw")
            mdw    *= muxn(self.dec_f3, [
                mulw, mulw, mulw, mulw,                                       # 0 MULW (1-3 unused)
                mux(zero_bw, 0xFFFF_FFFF,
                    mux(negmaxw, md_a32, md_a32.sdiv(md_b32), "divw_ovf"), "divw_z"),   # 4 DIVW
                mux(zero_bw, 0xFFFF_FFFF, md_a32 / md_b32, "divuw_z"),        # 5 DIVUW
                mux(zero_bw, md_a32, mux(negmaxw, 0, md_a32.srem(md_b32), "remw_ovf"), "remw_z"),  # 6 REMW
                mux(zero_bw, md_a32, md_a32 % md_b32, "remuw_z"),             # 7 REMUW
            ], "mdw_mux")
            self.mdw_out64 = sext(mdw, XLEN, "mdw_out64")

        # ---- branch condition ----
        br_eq  = self.rs1_val == self.rs2_val
        br_lt  = self.rs1_val.slt(self.rs2_val)
        br_ltu = self.rs1_val < self.rs2_val
        br_taken = wire(1, "br_taken")
        br_taken *= muxn(self.dec_f3, [
            br_eq, br_eq.lnot(), 0, 0,                               # BEQ/BNE (2,3 unused)
            br_lt, br_lt.lnot(), br_ltu, br_ltu.lnot(),              # BLT/BGE/BLTU/BGEU
        ], "br_mux")

        # ---- opcode legality + control-transfer flag ----
        self.op_unknown *= or_reduce(
            [self.dec_op == op for op in isa.known_opcodes(cfg)]).lnot()
        self.pc_redirect *= or_reduce([
            self.dec_op == isa.OP_JAL,
            self.dec_op == isa.OP_JALR,
            (self.dec_op == isa.OP_BRANCH).land(br_taken),
        ])

        # ---- CSR file / trap logic ----
        if cfg.ext_zicsr:
            self.csr.build_comb(self)

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
        if cfg.ext_m:
            with zelif((self.dec_op == isa.OP_OP).land(self.is_md)):
                self.wb_val *= self.md_out
            with zelif((self.dec_op == isa.OP_OP32).land(self.is_md)):
                self.wb_val *= self.mdw_out64
        if cfg.ext_a:
            with zelif(self.dec_op == isa.OP_AMO):
                self.wb_val *= mux(self.amo_is_sc, self.sc_fail.extend(XLEN),
                                   self.load_val, "amo_wb")
        with zelif(self.dec_op == isa.OP_IMM32):
            self.wb_val *= alu_w64
        with zelif(self.dec_op == isa.OP_OP32):
            self.wb_val *= alu_w64
        if cfg.ext_zicsr:
            with zelif(self.dec_op == isa.OP_SYSTEM):
                self.wb_val *= self.csr.csr_rdata                     # old CSR value
        with zelse():
            self.wb_val *= alu_out                                    # OP / OP_IMM

        wb_srcs = [self.dec_op == op for op in isa.WB_OPCODES]
        if cfg.ext_zicsr:
            wb_srcs.append(self.csr.is_csr)
        if cfg.ext_a:
            wb_srcs.append(self.is_amo)
        self.wb_en *= or_reduce(wb_srcs)

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
        ea_off = wire(XLEN, "ea_off")
        with zif(self.is_store == 1):
            ea_off *= imm_s
        if cfg.ext_a:
            with zelif(self.is_amo == 1):
                ea_off *= 0                           # AMOs address rs1 directly
        with zelse():
            ea_off *= imm_i
        self.mem_addr_c *= self.rs1_val + ea_off
        self.mem_size_c *= ins[13, 12]

        # ---- A extension: LR/SC + AMO ops ----
        if cfg.ext_a:
            amo_f5 = wire(5, "amo_f5"); amo_f5 *= ins[31, 27]
            self.is_amo    *= self.dec_op == isa.OP_AMO
            self.amo_is_lr *= amo_f5 == 0x02
            self.amo_is_sc *= amo_f5 == 0x03
            self.sc_fail   *= (self.resv.land(self.resv_addr == self.mem_addr_c)).lnot()

            # operate on sign-extended views (load_val already sexts W loads);
            # unsigned W compares use the zero-extended low words instead
            is_w   = self.dec_f3 == 2
            amo_ld = self.load_val
            amo_rs = mux(is_w, sext(self.rs2_val[31, 0], XLEN, "amo_rs_w"),
                         self.rs2_val, "amo_rs")
            u_ld   = mux(is_w, amo_ld[31, 0].extend(XLEN), amo_ld, "amo_uld")
            u_rs   = mux(is_w, self.rs2_val[31, 0].extend(XLEN), self.rs2_val, "amo_urs")

            AMO_FN = [
                (0x00, amo_ld + amo_rs),                                  # AMOADD
                (0x01, amo_rs),                                           # AMOSWAP
                (0x04, amo_ld ^ amo_rs),                                  # AMOXOR
                (0x08, amo_ld | amo_rs),                                  # AMOOR
                (0x0C, amo_ld & amo_rs),                                  # AMOAND
                (0x10, mux(amo_ld.slt(amo_rs), amo_ld, amo_rs, "amomin")),  # AMOMIN
                (0x14, mux(amo_ld.slt(amo_rs), amo_rs, amo_ld, "amomax")),  # AMOMAX
                (0x18, mux(u_ld < u_rs, amo_ld, amo_rs, "amominu")),      # AMOMINU
                (0x1C, mux(u_ld < u_rs, amo_rs, amo_ld, "amomaxu")),      # AMOMAXU
            ]
            first = True
            for f5, res in AMO_FN:
                blk = zif(amo_f5 == f5) if first else zelif(amo_f5 == f5)
                first = False
                with blk:
                    self.amo_result *= res

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
    def _wb_step(self):
        # Standard write-back: rf write (gated by wb_en, x0 excluded), pc advance,
        # CSR commit strobe + instret. Without Zicsr, unknown opcodes halt instead.
        with par():
            self.rf.cus_dynamic_assign(
                [slice(1, 32)], {"x": self.wb_val},
                lambda v: self.wb_en & (self.dec_rd == v.coord[0]),
            )
            self.pc |= self.next_pc
            if self.cfg.ext_zicsr:
                self.csr.csr_strobe  *= 1          # state-gated: high in this cycle only
                self.csr.minstret    |= self.csr.minstret + 1
            else:
                self.halted |= self.halted | self.op_unknown

    def _bus_read(self, addr, size):
        with par():
            self.b_req  |= 1
            self.b_we   |= 0
            self.b_addr |= addr
            self.b_size |= size
        scwait(self.b_ack)

    @flow
    def fsm(self):
        cfg = self.cfg
        with seq():
            with cwhile(self.halted == 0):
                # FETCH: issue a 4-byte read at pc
                self._bus_read(self.pc, 2)
                with par():
                    self.instr |= self.b_rdata[31, 0]
                    self.b_req |= 0
                # decode settle: the cif chain below samples its conditions at the
                # posedge that latched `instr`, so give the comb decode one cycle
                sywait(1)

                # dispatch: trap / mret preempt; loads & stores add a bus access
                if cfg.ext_zicsr:
                    with cif(self.csr.trap_now == 1):
                        with par():
                            self.csr.mepc   |= self.pc
                            self.csr.mcause |= self.csr.trap_cause
                            self.csr.mtval  |= self.csr.trap_tval
                            self.csr.mpie   |= self.csr.mie_g
                            self.csr.mie_g  |= 0
                            self.pc         |= self.csr.trap_target
                    with cselif(self.csr.is_mret == 1):
                        with par():
                            self.pc           |= self.csr.mepc
                            self.csr.mie_g    |= self.csr.mpie
                            self.csr.mpie     |= 1
                            self.csr.minstret |= self.csr.minstret + 1
                    with cselif(self.is_load == 1):
                        self._bus_read(self.mem_addr_c, self.mem_size_c)
                        with par():
                            self.ld_raw |= self.b_rdata
                            self.b_req  |= 0
                        self._wb_step()
                    with cselif(self.is_store == 1):
                        with par():
                            self.b_req   |= 1
                            self.b_we    |= 1
                            self.b_addr  |= self.mem_addr_c
                            self.b_wdata |= self.rs2_val
                            self.b_size  |= self.mem_size_c
                        scwait(self.b_ack)
                        self.b_req |= 0
                        self._wb_step()
                    if cfg.ext_a:
                        with cselif((self.is_amo & self.amo_is_lr) == 1):
                            self._bus_read(self.mem_addr_c, self.mem_size_c)
                            with par():
                                self.ld_raw    |= self.b_rdata
                                self.b_req     |= 0
                                self.resv      |= 1
                                self.resv_addr |= self.mem_addr_c
                            self._wb_step()
                        with cselif((self.is_amo & self.amo_is_sc) == 1):
                            # NOTE: _wb_step samples sc_fail, so the reservation
                            # is cleared one step AFTER write-back, not before.
                            with cif(self.sc_fail == 0):
                                with par():
                                    self.b_req   |= 1
                                    self.b_we    |= 1
                                    self.b_addr  |= self.mem_addr_c
                                    self.b_wdata |= self.rs2_val
                                    self.b_size  |= self.mem_size_c
                                scwait(self.b_ack)
                                self.b_req |= 0
                                self._wb_step()
                                self.resv |= 0
                            with cselse():
                                self._wb_step()
                                self.resv |= 0
                        with cselif(self.is_amo == 1):          # read-modify-write AMOs
                            self._bus_read(self.mem_addr_c, self.mem_size_c)
                            with par():
                                self.ld_raw |= self.b_rdata
                                self.b_req  |= 0
                            with par():
                                self.b_req   |= 1
                                self.b_we    |= 1
                                self.b_addr  |= self.mem_addr_c
                                self.b_wdata |= self.amo_result
                                self.b_size  |= self.mem_size_c
                            scwait(self.b_ack)
                            self.b_req |= 0
                            self._wb_step()
                    with cselse():
                        self._wb_step()
                else:
                    with cif(self.is_load == 1):
                        self._bus_read(self.mem_addr_c, self.mem_size_c)
                        with par():
                            self.ld_raw |= self.b_rdata
                            self.b_req  |= 0
                        self._wb_step()
                    with cselif(self.is_store == 1):
                        with par():
                            self.b_req   |= 1
                            self.b_we    |= 1
                            self.b_addr  |= self.mem_addr_c
                            self.b_wdata |= self.rs2_val
                            self.b_size  |= self.mem_size_c
                        scwait(self.b_ack)
                        self.b_req |= 0
                        self._wb_step()
                    with cselse():
                        self._wb_step()
