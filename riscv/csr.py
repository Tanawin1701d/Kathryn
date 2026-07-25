# Machine-mode CSR file + trap logic for RV64Core. Not a Module — an
# elaboration helper: `declare()` runs inside the core's @init scope,
# `build_comb()` inside its comb @flow section. The FSM triggers CSR writes via
# a state-strobe wire (`csr_strobe`), so the write chain lives entirely in comb
# scope and fires at the write-back posedge only.

from __future__ import annotations

from kathryn import reg, wire, val, zif, zelif, zelse
from kathryn.lib import cat, mux, or_reduce

# ---- CSR addresses -----------------------------------------------------------
MSTATUS, MISA, MIE, MTVEC       = 0x300, 0x301, 0x304, 0x305
MSCRATCH, MEPC, MCAUSE, MTVAL   = 0x340, 0x341, 0x342, 0x343
MIP                             = 0x344
MCYCLE, MINSTRET                = 0xB00, 0xB02
CYCLE, INSTRET                  = 0xC00, 0xC02   # read-only user aliases
MVENDORID, MARCHID, MIMPID      = 0xF11, 0xF12, 0xF13
MHARTID                         = 0xF14

# ---- trap causes -------------------------------------------------------------
EXC_MISALIGNED_FETCH = 0
EXC_ILLEGAL          = 2
EXC_BREAKPOINT       = 3
EXC_ECALL_M          = 11
IRQ_MSI, IRQ_MTI, IRQ_MEI = 3, 7, 11
IRQ_BIT = 1 << 63


class CsrFile:
    def __init__(self, cfg):
        self.cfg = cfg

    # ---- state (call from @init) --------------------------------------------
    def declare(self):
        self.mie_g    = reg(1,  "mstatus_mie")    # mstatus.MIE
        self.mpie     = reg(1,  "mstatus_mpie")   # mstatus.MPIE
        self.msie     = reg(1,  "mie_msie")
        self.mtie     = reg(1,  "mie_mtie")
        self.meie     = reg(1,  "mie_meie")
        self.mtvec    = reg(64, "mtvec")
        self.mscratch = reg(64, "mscratch")
        self.mepc     = reg(64, "mepc")
        self.mcause   = reg(64, "mcause")
        self.mtval    = reg(64, "mtval")
        self.mcycle   = reg(64, "mcycle")
        self.minstret = reg(64, "minstret")

    # ---- comb logic (call from the comb @flow section) ----------------------
    # `core` supplies: instr, dec_f3, dec_rs1, rs1_val, and the irq pins.
    def build_comb(self, core):
        cfg = self.cfg
        ins = core.instr

        for r in (self.mie_g, self.mpie, self.msie, self.mtie, self.meie,
                  self.mtvec, self.mscratch, self.mepc, self.mcause, self.mtval,
                  self.mcycle, self.minstret):
            r.reset(0)

        # free-running cycle counter (constant-true zif: bare clocked assigns panic)
        one = val(1, 1, "const_one")
        with zif(one == 1):
            self.mcycle |= self.mcycle + 1

        # ---- composed read views -------------------------------------------
        misa_val = ((2 << 62) | (1 << 8)                      # MXL=64, I
                    | ((1 << 12) if cfg.ext_m else 0)
                    | ((1 << 0)  if cfg.ext_a else 0))
        mstatus_rd = cat(val(51, 0), val(2, 3), val(3, 0), self.mpie,
                         val(3, 0), self.mie_g, val(3, 0), name="mstatus_rd")
        mie_rd     = cat(val(52, 0), self.meie, val(3, 0), self.mtie,
                         val(3, 0), self.msie, val(3, 0), name="mie_rd")
        mip_rd     = cat(val(52, 0), core.irq_meip, val(3, 0), core.irq_mtip,
                         val(3, 0), core.irq_msip, val(3, 0), name="mip_rd")

        # ---- SYSTEM-opcode sub-decode --------------------------------------
        imm12    = wire(12, "sys_imm12");  imm12    *= ins[31, 20]
        self.csr_addr = imm12
        is_sys   = core.dec_op == 0x73
        f3       = core.dec_f3
        is_priv  = is_sys.land(f3 == 0)
        self.is_ecall  = wire(1, "is_ecall");  self.is_ecall  *= is_priv.land(imm12 == 0x000)
        self.is_ebreak = wire(1, "is_ebreak"); self.is_ebreak *= is_priv.land(imm12 == 0x001)
        self.is_mret   = wire(1, "is_mret");   self.is_mret   *= is_priv.land(imm12 == 0x302)
        is_wfi         = is_priv.land(imm12 == 0x105)          # WFI executes as a nop
        self.is_csr    = wire(1, "is_csr")
        self.is_csr   *= is_sys.land((f3 == 4).lor(f3 == 0).lnot())

        # ---- CSR read mux ---------------------------------------------------
        rdata = wire(64, "csr_rdata")
        self.csr_rdata = rdata
        RD = [
            (MSTATUS, mstatus_rd), (MISA, misa_val), (MIE, mie_rd),
            (MTVEC, self.mtvec), (MSCRATCH, self.mscratch), (MEPC, self.mepc),
            (MCAUSE, self.mcause), (MTVAL, self.mtval), (MIP, mip_rd),
            (MCYCLE, self.mcycle), (MINSTRET, self.minstret),
            (CYCLE, self.mcycle), (INSTRET, self.minstret),
            (MVENDORID, 0), (MARCHID, 0), (MIMPID, 0), (MHARTID, 0),
        ]
        first = True
        for addr, src in RD:
            blk = zif(imm12 == addr) if first else zelif(imm12 == addr)
            first = False
            with blk:
                if isinstance(src, int):
                    rdata *= val(64, src)
                else:
                    rdata *= src

        known = wire(1, "csr_known")
        known *= or_reduce([imm12 == a for a, _ in RD])

        # ---- CSR write value -----------------------------------------------
        # base operand: rs1 value or the zero-extended rs1 field (immediate forms)
        zimm  = core.dec_rs1.extend(64)
        base  = mux(f3[2], zimm, core.rs1_val, "csr_base")
        wval  = wire(64, "csr_wval")
        with zif(f3[1, 0] == 1):
            wval *= base                                       # CSRRW[I]
        with zelif(f3[1, 0] == 2):
            wval *= rdata | base                               # CSRRS[I]
        with zelse():
            wval *= rdata & ~base                              # CSRRC[I]
        self.csr_wval = wval

        # set/clear with rs1=x0 (or zimm=0) must not write; read-only region 0xCxx/0xFxx
        wen  = wire(1, "csr_wen")
        wen *= self.is_csr.land(
            mux((f3[1, 0] == 1).lnot().land(core.dec_rs1 == 0), val(1, 0), val(1, 1), "csr_wen_mux") == 1)
        readonly = imm12[11, 10] == 3
        self.csr_illegal = wire(1, "csr_illegal")
        self.csr_illegal *= self.is_csr.land(known.lnot().lor(wen.land(readonly)))

        # ---- exceptions / interrupts ---------------------------------------
        bad_priv = is_priv.land(or_reduce(
            [self.is_ecall, self.is_ebreak, self.is_mret, is_wfi]).lnot())
        self.is_illegal = wire(1, "is_illegal")
        self.is_illegal *= core.op_unknown.lor(bad_priv).lor(self.csr_illegal)
        self.is_wfi = is_wfi

        irq_src = wire(1, "irq_any")
        irq_src *= self.mie_g & ((self.meie & core.irq_meip)
                                 | (self.mtie & core.irq_mtip)
                                 | (self.msie & core.irq_msip))
        self.irq_pending = irq_src
        irq_code = wire(4, "irq_code")                          # MEI > MSI > MTI
        with zif((self.meie & core.irq_meip) == 1):
            irq_code *= IRQ_MEI
        with zelif((self.msie & core.irq_msip) == 1):
            irq_code *= IRQ_MSI
        with zelse():
            irq_code *= IRQ_MTI

        # instruction-address-misaligned on a taken control transfer (no C ext)
        ma_fetch = wire(1, "ma_fetch")
        if cfg.ext_c:
            ma_fetch *= 0
        else:
            ma_fetch *= (core.next_pc[1, 0] == 0).lnot().land(core.pc_redirect)

        # ---- trap bundle (consumed by the FSM's trap branch) ----------------
        self.trap_now = wire(1, "trap_now")
        self.trap_now *= or_reduce([irq_src, self.is_illegal, self.is_ecall,
                                    self.is_ebreak, ma_fetch])

        cause = wire(64, "trap_cause")
        tval  = wire(64, "trap_tval")
        with zif(irq_src == 1):
            cause *= irq_code.extend(64) | val(64, IRQ_BIT)
            tval  *= 0
        with zelif(self.is_illegal == 1):
            cause *= EXC_ILLEGAL
            tval  *= ins.extend(64)
        with zelif(self.is_ebreak == 1):
            cause *= EXC_BREAKPOINT
            tval  *= core.pc
        with zelif(self.is_ecall == 1):
            cause *= EXC_ECALL_M
            tval  *= 0
        with zelse():
            cause *= EXC_MISALIGNED_FETCH
            tval  *= core.next_pc
        self.trap_cause = cause
        self.trap_tval  = tval

        # vectored mtvec (mode=1) offsets interrupts by 4*cause
        base_t  = self.mtvec & 0xFFFF_FFFF_FFFF_FFFC
        vec_off = (irq_code.extend(64)) << val(64, 2)
        self.trap_target = wire(64, "trap_target")
        self.trap_target *= mux((self.mtvec[1, 0] == 1).land(irq_src),
                                base_t + vec_off, base_t, "trap_tgt_mux")

        # ---- write-back strobe + write chain --------------------------------
        # csr_strobe is comb-assigned *inside* the FSM's WB step, so it is high
        # exactly during that state; the clocked writes below fire on its posedge.
        self.csr_strobe = wire(1, "csr_strobe")
        commit = wen.land(self.csr_illegal.lnot()).land(self.csr_strobe)
        with zif(commit.land(imm12 == MSTATUS) == 1):
            self.mie_g |= wval[3]
            self.mpie  |= wval[7]
        with zelif(commit.land(imm12 == MIE) == 1):
            self.msie |= wval[3]
            self.mtie |= wval[7]
            self.meie |= wval[11]
        with zelif(commit.land(imm12 == MTVEC) == 1):
            self.mtvec |= wval
        with zelif(commit.land(imm12 == MSCRATCH) == 1):
            self.mscratch |= wval
        with zelif(commit.land(imm12 == MEPC) == 1):
            self.mepc |= wval & 0xFFFF_FFFF_FFFF_FFFE
        with zelif(commit.land(imm12 == MCAUSE) == 1):
            self.mcause |= wval
        with zelif(commit.land(imm12 == MTVAL) == 1):
            self.mtval |= wval
        with zelif(commit.land(imm12 == MCYCLE) == 1):
            self.mcycle |= wval
        with zelif(commit.land(imm12 == MINSTRET) == 1):
            self.minstret |= wval
