use crate::model::common::identifier::Identifiable;
use crate::model::hw_component::common::hcp_assign::HcpAssignable;
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HwComponentType};
use crate::model::hw_component::common::hcp_read::HcpReadable;
use crate::model::hw_component::expression::Expression;
use crate::model::hw_component::memBlk::MemBlk;
use crate::model::hw_component::memEle::MemEle;
use crate::model::hw_component::reg::Reg;
use crate::model::hw_component::val::Val;
use crate::model::hw_component::wire::Wire;
use crate::model::hw_component::io_wire::IoWire;
use crate::model::hw_component::sp_reg::cnt_reg::CntReg;
use crate::model::hw_component::sp_reg::sync_reg::SyncReg;
use crate::model::hw_component::sp_reg::state_reg::StateReg;
use crate::model::hw_component::sp_reg::wait_reg::{CondWaitStateReg, CycleWaitStateReg};
use crate::model::model_arena::ModelArena;

macro_rules! dispatch_hcp {
    ($self:expr, $hcpIdent:expr, $method:ident) => {{
        let handle = *$hcpIdent.get_arena_handle();
        match $hcpIdent.get_hw_type() {
            HwComponentType::Reg             => $self.regs       .$method(handle),
            HwComponentType::Wire            => $self.wires      .$method(handle),
            HwComponentType::IoWire          => $self.io_wires   .$method(handle),
            HwComponentType::Val             => $self.vals       .$method(handle),
            HwComponentType::MemBlockIndexer => $self.mem_eles   .$method(handle),
            HwComponentType::Expression      => $self.expressions.$method(handle),
            HwComponentType::StateReg        => $self.state_regs .$method(handle),
            HwComponentType::SyncReg         => $self.sync_regs  .$method(handle),
            HwComponentType::CntReg          => $self.cnt_regs   .$method(handle),
            HwComponentType::CondWaitStateReg   => $self.cond_wait_regs .$method(handle),
            HwComponentType::CycleWaitStateReg  => $self.cycle_wait_regs.$method(handle),
            t => panic!("HwComponentType {:?} is not HCP-accessible", t),
        }
    }};
}


impl ModelArena {
    // -----------------------------------------------------------------------
    // HCP inserts (return HcpIdent; arena_handle stamped via post-insert read)
    // -----------------------------------------------------------------------
    pub fn add_reg      (&mut self, r: Reg)         -> HcpIdent { let h = self.regs       .insert(r); self.regs       .get(h).get_ident() }
    pub fn add_wire     (&mut self, w: Wire)        -> HcpIdent { let h = self.wires      .insert(w); self.wires      .get(h).get_ident() }
    pub fn add_io_wire  (&mut self, w: IoWire)      -> HcpIdent { let h = self.io_wires   .insert(w); self.io_wires   .get(h).get_ident() }
    pub fn add_val      (&mut self, v: Val)         -> HcpIdent { let h = self.vals       .insert(v); self.vals       .get(h).get_ident() }
    pub fn add_mem_ele  (&mut self, e: MemEle)      -> HcpIdent { let h = self.mem_eles   .insert(e); self.mem_eles   .get(h).get_ident() }
    pub fn add_mem_blk  (&mut self, b: MemBlk)      -> HcpIdent { let h = self.mem_blks   .insert(b); self.mem_blks   .get(h).get_ident() }
    pub fn add_expression(&mut self, e: Expression) -> HcpIdent { let h = self.expressions.insert(e); self.expressions.get(h).get_ident() }
    pub fn add_state_reg(&mut self, s: StateReg)    -> HcpIdent { let h = self.state_regs .insert(s); self.state_regs .get(h).get_ident() }
    pub fn add_sync_reg (&mut self, s: SyncReg)     -> HcpIdent { let h = self.sync_regs  .insert(s); self.sync_regs  .get(h).get_ident() }
    pub fn add_cnt_reg  (&mut self, c: CntReg)      -> HcpIdent { let h = self.cnt_regs   .insert(c); self.cnt_regs   .get(h).get_ident() }
    pub fn add_cond_wait_reg (&mut self, c: CondWaitStateReg)  -> HcpIdent { let h = self.cond_wait_regs .insert(c); self.cond_wait_regs .get(h).get_ident() }
    pub fn add_cycle_wait_reg(&mut self, c: CycleWaitStateReg) -> HcpIdent { let h = self.cycle_wait_regs.insert(c); self.cycle_wait_regs.get(h).get_ident() }

    // -----------------------------------------------------------------------
    // HCP trait-object dispatch (cannot be expressed via take/replace_back
    // because the borrow targets a trait object inside the arena)
    // -----------------------------------------------------------------------
    pub fn get_hcp_assign      (&self,     ident: &HcpIdent) -> &    dyn HcpAssignable { dispatch_hcp!(self, ident, get    ) }
    pub fn get_hcp_assign_mut  (&mut self, ident: &HcpIdent) -> &mut dyn HcpAssignable { dispatch_hcp!(self, ident, get_mut) }
    pub fn get_hcp_readable    (&self,     ident: &HcpIdent) -> &    dyn HcpReadable   { dispatch_hcp!(self, ident, get    ) }
    pub fn get_hcp_readable_mut(&mut self, ident: &HcpIdent) -> &mut dyn HcpReadable   { dispatch_hcp!(self, ident, get_mut) }

    pub fn borrow_asb_mut(&mut self, ident: HcpIdent) -> &mut dyn HcpAssignable {
        self.get_hcp_assign_mut(&ident)
    }

    pub fn get_hw_bit_sz(&self, ident: &HcpIdent) -> i32 {
        self.get_hcp_assign(ident).get_des_slice().get_size()
    }

    // -----------------------------------------------------------------------
    // HCP take / replace_back — the canonical owned-access path. Use these
    // instead of typed get/get_mut.
    // -----------------------------------------------------------------------
    pub fn take_reg      (&mut self, h: HcpIdent) -> Reg        { self.regs       .take(*h.get_arena_handle()) }
    pub fn take_wire     (&mut self, h: HcpIdent) -> Wire       { self.wires      .take(*h.get_arena_handle()) }
    pub fn take_io_wire  (&mut self, h: HcpIdent) -> IoWire     { self.io_wires   .take(*h.get_arena_handle()) }
    pub fn take_val      (&mut self, h: HcpIdent) -> Val        { self.vals       .take(*h.get_arena_handle()) }
    pub fn take_mem_ele  (&mut self, h: HcpIdent) -> MemEle     { self.mem_eles   .take(*h.get_arena_handle()) }
    pub fn take_mem_blk  (&mut self, h: HcpIdent) -> MemBlk     { self.mem_blks   .take(*h.get_arena_handle()) }
    pub fn take_expression(&mut self, h: HcpIdent) -> Expression { self.expressions.take(*h.get_arena_handle()) }
    pub fn take_state_reg(&mut self, h: HcpIdent) -> StateReg   { self.state_regs .take(*h.get_arena_handle()) }
    pub fn take_sync_reg (&mut self, h: HcpIdent) -> SyncReg    { self.sync_regs  .take(*h.get_arena_handle()) }
    pub fn take_cnt_reg  (&mut self, h: HcpIdent) -> CntReg     { self.cnt_regs   .take(*h.get_arena_handle()) }
    pub fn take_cond_wait_reg (&mut self, h: HcpIdent) -> CondWaitStateReg  { self.cond_wait_regs .take(*h.get_arena_handle()) }
    pub fn take_cycle_wait_reg(&mut self, h: HcpIdent) -> CycleWaitStateReg { self.cycle_wait_regs.take(*h.get_arena_handle()) }

    pub fn replace_back_reg      (&mut self, v: Reg)        { let h = *v.get_arena_handle(); self.regs       .replace_back(h, v) }
    pub fn replace_back_wire     (&mut self, v: Wire)       { let h = *v.get_arena_handle(); self.wires      .replace_back(h, v) }
    pub fn replace_back_io_wire  (&mut self, v: IoWire)    { let h = *v.get_arena_handle(); self.io_wires   .replace_back(h, v) }
    pub fn replace_back_val      (&mut self, v: Val)        { let h = *v.get_arena_handle(); self.vals       .replace_back(h, v) }
    pub fn replace_back_mem_ele  (&mut self, v: MemEle)     { let h = *v.get_arena_handle(); self.mem_eles   .replace_back(h, v) }
    pub fn replace_back_mem_blk  (&mut self, v: MemBlk)     { let h = *v.get_arena_handle(); self.mem_blks   .replace_back(h, v) }
    pub fn replace_back_expression(&mut self, v: Expression) { let h = *v.get_arena_handle(); self.expressions.replace_back(h, v) }
    pub fn replace_back_state_reg(&mut self, v: StateReg)   { let h = *v.get_arena_handle(); self.state_regs .replace_back(h, v) }
    pub fn replace_back_sync_reg (&mut self, v: SyncReg)    { let h = *v.get_arena_handle(); self.sync_regs  .replace_back(h, v) }
    pub fn replace_back_cnt_reg  (&mut self, v: CntReg)     { let h = *v.get_arena_handle(); self.cnt_regs   .replace_back(h, v) }
    pub fn replace_back_cond_wait_reg (&mut self, v: CondWaitStateReg)  { let h = *v.get_arena_handle(); self.cond_wait_regs .replace_back(h, v) }
    pub fn replace_back_cycle_wait_reg(&mut self, v: CycleWaitStateReg) { let h = *v.get_arena_handle(); self.cycle_wait_regs.replace_back(h, v) }
}
