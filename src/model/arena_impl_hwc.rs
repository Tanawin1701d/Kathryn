use crate::model::common::identifier::Identifiable;
use crate::model::hw_component::common::hcp_assign::{HcpAssignable, HcpIoMark};
use crate::model::hw_component::common::hcp_base::HcpBase;
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HwComponentType};
use crate::model::hw_component::common::slice::Slice;
use crate::model::hw_component::expression::Expression;
use crate::model::hw_component::mem_blk::MemBlk;
use crate::model::hw_component::mem_ele::MemEle;
use crate::model::hw_component::reg::Reg;
use crate::model::hw_component::val::Val;
use crate::model::hw_component::wire::Wire;
use crate::model::hw_component::io_wire::IoWire;
use crate::model::hw_component::sp_reg::cnt_reg::CntReg;
use crate::model::hw_component::sp_reg::sync_reg::SyncReg;
use crate::model::hw_component::sp_reg::state_reg::StateReg;
use crate::model::hw_component::sp_reg::wait_reg::{CondWaitStateReg, CycleWaitStateReg};
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::NcpIdent;

// dispatch_hcp!: two forms.
//   dispatch_hcp!(self, ident, method) — 11 HCP-assignable types (MemBlock excluded);
//                                        returns &dyn / &mut dyn HcpAssignable.
//   dispatch_hcp!(take self, ident)    — all 12 HcpBase types incl. MemBlock;
//                                        returns Box<dyn HcpBase> (owned).
macro_rules! dispatch_hcp {
    ($self:expr, $hcpIdent:expr, $method:ident) => {{
        let handle = *$hcpIdent.get_arena_handle();
        match $hcpIdent.get_hw_type() {
            HwComponentType::Reg               => $self.regs            .$method(handle),
            HwComponentType::Wire              => $self.wires           .$method(handle),
            HwComponentType::IoWire            => $self.io_wires        .$method(handle),
            HwComponentType::Val               => $self.vals            .$method(handle),
            HwComponentType::MemBlockIndexer   => $self.mem_eles        .$method(handle),
            HwComponentType::Expression        => $self.expressions     .$method(handle),
            HwComponentType::StateReg          => $self.state_regs      .$method(handle),
            HwComponentType::SyncReg           => $self.sync_regs       .$method(handle),
            HwComponentType::CntReg            => $self.cnt_regs        .$method(handle),
            HwComponentType::CondWaitStateReg  => $self.cond_wait_regs  .$method(handle),
            HwComponentType::CycleWaitStateReg => $self.cycle_wait_regs .$method(handle),
            HwComponentType::MemBlock          => $self.mem_blks        .$method(handle),
            t => panic!("HwComponentType {:?} is not HCP-assignable", t),
        }
    }};
    (take $self:expr, $hcpIdent:expr) => {{
        let handle = *$hcpIdent.get_arena_handle();
        match $hcpIdent.get_hw_type() {
            HwComponentType::Reg               => Box::new($self.regs            .take(handle)) as Box<dyn HcpBase>,
            HwComponentType::Wire              => Box::new($self.wires           .take(handle)) as Box<dyn HcpBase>,
            HwComponentType::IoWire            => Box::new($self.io_wires        .take(handle)) as Box<dyn HcpBase>,
            HwComponentType::Val               => Box::new($self.vals            .take(handle)) as Box<dyn HcpBase>,
            HwComponentType::MemBlockIndexer   => Box::new($self.mem_eles        .take(handle)) as Box<dyn HcpBase>,
            HwComponentType::Expression        => Box::new($self.expressions     .take(handle)) as Box<dyn HcpBase>,
            HwComponentType::StateReg          => Box::new($self.state_regs      .take(handle)) as Box<dyn HcpBase>,
            HwComponentType::SyncReg           => Box::new($self.sync_regs       .take(handle)) as Box<dyn HcpBase>,
            HwComponentType::CntReg            => Box::new($self.cnt_regs        .take(handle)) as Box<dyn HcpBase>,
            HwComponentType::CondWaitStateReg  => Box::new($self.cond_wait_regs  .take(handle)) as Box<dyn HcpBase>,
            HwComponentType::CycleWaitStateReg => Box::new($self.cycle_wait_regs .take(handle)) as Box<dyn HcpBase>,
            HwComponentType::MemBlock          => Box::new($self.mem_blks        .take(handle)) as Box<dyn HcpBase>,
            t => panic!("HwComponentType {:?} is not HcpBase", t),
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
    pub fn get_hcp_assign(&self, ident: &HcpIdent) -> &dyn HcpAssignable { dispatch_hcp!(self, ident, get) }

    pub fn get_hw_bit_sz(&self, ident: &HcpIdent) -> i32 {
        self.get_hw_slice(ident).get_size()
    }

    pub fn get_hw_slice(&self, ident: &HcpIdent) -> Slice {
        self.get_hcp_assign(ident).get_des_slice()
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

    // -----------------------------------------------------------------------
    // Polymorphic take/replace_back — take uses the macro (ONE match location);
    // replace_back is zero-match via HcpBase::replace_back_into_arena.
    // -----------------------------------------------------------------------
    pub fn take_hcp    (&mut self, hcp_i: HcpIdent) -> Box<dyn HcpBase> { dispatch_hcp!(take self, hcp_i) }
    pub fn replace_back_hcp(&mut self, v: Box<dyn HcpBase>)              { v.replace_back_into_arena(self); }


    // Generate an assignment node writing src_i into des_i: take the destination
    // HCP out (freeing the arena for do_asm), let it build its own asm node, put it back.
    pub fn gen_asm_node(&mut self, des_i    : HcpIdent     , src_i    : HcpIdent,
                                   des_slice: Option<Slice>, src_slice: Slice   ,
    ) -> NcpIdent {
        let des= self.take_hcp(des_i);
        let node_i = des.do_asm(src_i, des_slice, src_slice, self);
        self.replace_back_hcp(des);
        node_i
    }

    // Build a basic assignment node and attach it where the build is currently
    // pointing (active flow block, or the top module if no block is building).
    pub fn gen_basic_assign(&mut self, des_i    : HcpIdent     , src_i    : HcpIdent,
                                       des_slice: Option<Slice>, src_slice: Slice   ,
    ) {
        let node_i = self.gen_asm_node(des_i, src_i, des_slice, src_slice);
        self.attach_basic_node_to_current_scope(node_i);
    }

    // ---- IO mark ----

    // Stamp an HCP as an IO port (direction + user-facing name). Take/replace_back
    // because mark_as_io needs &mut and the dispatch borrow targets a trait object.
    pub fn mark_as_io(&mut self, hcp_i: HcpIdent, is_input: bool, io_name: String) {
        let mut hcp = self.take_hcp(hcp_i);
        hcp.mark_as_io(is_input, io_name);
        self.replace_back_hcp(hcp);
    }

    // True if the HCP has been marked as an IO port.
    pub fn is_marked_as_io(&self, hcp_i: &HcpIdent) -> bool {
        self.get_hcp_assign(hcp_i).is_marked_as_io()
    }

    // Read the HCP's IO mark (direction + name); None if not marked as IO.
    pub fn get_io_mark(&self, hcp_i: &HcpIdent) -> Option<&HcpIoMark> {
        self.get_hcp_assign(hcp_i).get_io_mark()
    }




}
