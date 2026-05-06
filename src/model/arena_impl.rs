use crate::common::arena_base::ArenaGroup;
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
use crate::model::hw_component::sp_reg::cnt_reg::CntReg;
use crate::model::hw_component::sp_reg::sync_reg::SyncReg;
use crate::model::hw_component::sp_reg::state_reg::StateReg;
use crate::model::hw_component::sp_reg::wait_reg::{CondWaitStateReg, CycleWaitStateReg};
use crate::model::model_arena::ModelArena;
use crate::model::module::module::Module;
use crate::model::module::module_ident::ModuleIdent;

macro_rules! dispatch_hcp {
    ($self:expr, $hcpIdent:expr, $method:ident) => {{
        let handle = *$hcpIdent.get_arena_handle();
        match $hcpIdent.get_hw_type() {
            HwComponentType::Reg             => $self.regs       .$method(handle),
            HwComponentType::Wire            => $self.wires      .$method(handle),
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
    pub fn new() -> Self {
        Self {
            regs       : ArenaGroup::new(),
            wires      : ArenaGroup::new(),
            vals       : ArenaGroup::new(),
            mem_eles   : ArenaGroup::new(),
            mem_blks   : ArenaGroup::new(),
            expressions: ArenaGroup::new(),
            state_regs : ArenaGroup::new(),
            sync_regs  : ArenaGroup::new(),
            cnt_regs   : ArenaGroup::new(),
            cond_wait_regs : ArenaGroup::new(),
            cycle_wait_regs: ArenaGroup::new(),
            ue_basics  : ArenaGroup::new(),
            ue_grps    : ArenaGroup::new(),
            ue_conds   : ArenaGroup::new(),
            ue_switches: ArenaGroup::new(),
            asm_nodes        : ArenaGroup::new(),
            state_nodes      : ArenaGroup::new(),
            syn_nodes        : ArenaGroup::new(),
            wait_cond_nodes  : ArenaGroup::new(),
            wait_cycle_nodes : ArenaGroup::new(),
            counter_nodes    : ArenaGroup::new(),
            pseudo_nodes     : ArenaGroup::new(),
            dummy_nodes      : ArenaGroup::new(),
            opr_nodes        : ArenaGroup::new(),
            modules          : ArenaGroup::new(),
        }
    }

    pub fn reset(&mut self) {
        self.regs        = ArenaGroup::new();
        self.wires       = ArenaGroup::new();
        self.vals        = ArenaGroup::new();
        self.mem_eles    = ArenaGroup::new();
        self.mem_blks    = ArenaGroup::new();
        self.expressions = ArenaGroup::new();
        self.state_regs  = ArenaGroup::new();
        self.sync_regs   = ArenaGroup::new();
        self.cnt_regs    = ArenaGroup::new();
        self.cond_wait_regs  = ArenaGroup::new();
        self.cycle_wait_regs = ArenaGroup::new();
        self.ue_basics   = ArenaGroup::new();
        self.ue_grps     = ArenaGroup::new();
        self.ue_conds    = ArenaGroup::new();
        self.ue_switches = ArenaGroup::new();
        self.asm_nodes        = ArenaGroup::new();
        self.state_nodes      = ArenaGroup::new();
        self.syn_nodes        = ArenaGroup::new();
        self.wait_cond_nodes  = ArenaGroup::new();
        self.wait_cycle_nodes = ArenaGroup::new();
        self.counter_nodes    = ArenaGroup::new();
        self.pseudo_nodes     = ArenaGroup::new();
        self.dummy_nodes      = ArenaGroup::new();
        self.opr_nodes        = ArenaGroup::new();
        self.modules          = ArenaGroup::new();
    }

    // -----------------------------------------------------------------------
    // HCP inserts (return raw ArenaHandle for use by make_* factory methods)
    // -----------------------------------------------------------------------
    pub fn add_reg      (&mut self, r: Reg)        -> HcpIdent { let h = self.regs       .insert(r); self.regs       .get(h).get_ident() }
    pub fn add_wire     (&mut self, w: Wire)       -> HcpIdent { let h = self.wires      .insert(w); self.wires      .get(h).get_ident() }
    pub fn add_val      (&mut self, v: Val)        -> HcpIdent { let h = self.vals       .insert(v); self.vals       .get(h).get_ident() }
    pub fn add_mem_ele  (&mut self, e: MemEle)     -> HcpIdent { let h = self.mem_eles   .insert(e); self.mem_eles   .get(h).get_ident() }
    pub fn add_mem_blk  (&mut self, b: MemBlk)     -> HcpIdent { let h = self.mem_blks   .insert(b); self.mem_blks   .get(h).get_ident() }
    pub fn add_expression(&mut self, e: Expression) -> HcpIdent { let h = self.expressions.insert(e); self.expressions.get(h).get_ident() }
    pub fn add_state_reg(&mut self, s: StateReg)   -> HcpIdent { let h = self.state_regs .insert(s); self.state_regs .get(h).get_ident() }
    pub fn add_sync_reg (&mut self, s: SyncReg)    -> HcpIdent { let h = self.sync_regs  .insert(s); self.sync_regs  .get(h).get_ident() }
    pub fn add_cnt_reg  (&mut self, c: CntReg)     -> HcpIdent { let h = self.cnt_regs   .insert(c); self.cnt_regs   .get(h).get_ident() }
    pub fn add_cond_wait_reg (&mut self, c: CondWaitStateReg)  -> HcpIdent { let h = self.cond_wait_regs .insert(c); self.cond_wait_regs .get(h).get_ident() }
    pub fn add_cycle_wait_reg(&mut self, c: CycleWaitStateReg) -> HcpIdent { let h = self.cycle_wait_regs.insert(c); self.cycle_wait_regs.get(h).get_ident() }

    // -----------------------------------------------------------------------
    // HCP getters
    // -----------------------------------------------------------------------
    pub fn get_reg      (&self, h: HcpIdent) -> &Reg        { self.regs       .get(*h.get_arena_handle()) }
    pub fn get_wire     (&self, h: HcpIdent) -> &Wire       { self.wires      .get(*h.get_arena_handle()) }
    pub fn get_val      (&self, h: HcpIdent) -> &Val        { self.vals       .get(*h.get_arena_handle()) }
    pub fn get_mem_ele  (&self, h: HcpIdent) -> &MemEle     { self.mem_eles   .get(*h.get_arena_handle()) }
    pub fn get_mem_blk  (&self, h: HcpIdent) -> &MemBlk     { self.mem_blks   .get(*h.get_arena_handle()) }
    pub fn get_expression(&self, h: HcpIdent) -> &Expression { self.expressions.get(*h.get_arena_handle()) }
    pub fn get_state_reg(&self, h: HcpIdent) -> &StateReg   { self.state_regs .get(*h.get_arena_handle()) }
    pub fn get_sync_reg (&self, h: HcpIdent) -> &SyncReg    { self.sync_regs  .get(*h.get_arena_handle()) }
    pub fn get_cnt_reg  (&self, h: HcpIdent) -> &CntReg     { self.cnt_regs   .get(*h.get_arena_handle()) }
    pub fn get_cond_wait_reg (&self, h: HcpIdent) -> &CondWaitStateReg  { self.cond_wait_regs .get(*h.get_arena_handle()) }
    pub fn get_cycle_wait_reg(&self, h: HcpIdent) -> &CycleWaitStateReg { self.cycle_wait_regs.get(*h.get_arena_handle()) }

    pub fn get_reg_mut      (&mut self, h: HcpIdent) -> &mut Reg        { self.regs       .get_mut(*h.get_arena_handle()) }
    pub fn get_wire_mut     (&mut self, h: HcpIdent) -> &mut Wire       { self.wires      .get_mut(*h.get_arena_handle()) }
    pub fn get_val_mut      (&mut self, h: HcpIdent) -> &mut Val        { self.vals       .get_mut(*h.get_arena_handle()) }
    pub fn get_mem_ele_mut  (&mut self, h: HcpIdent) -> &mut MemEle     { self.mem_eles   .get_mut(*h.get_arena_handle()) }
    pub fn get_mem_blk_mut  (&mut self, h: HcpIdent) -> &mut MemBlk     { self.mem_blks   .get_mut(*h.get_arena_handle()) }
    pub fn get_expression_mut(&mut self, h: HcpIdent) -> &mut Expression { self.expressions.get_mut(*h.get_arena_handle()) }
    pub fn get_state_reg_mut(&mut self, h: HcpIdent) -> &mut StateReg   { self.state_regs .get_mut(*h.get_arena_handle()) }
    pub fn get_sync_reg_mut (&mut self, h: HcpIdent) -> &mut SyncReg    { self.sync_regs  .get_mut(*h.get_arena_handle()) }
    pub fn get_cnt_reg_mut  (&mut self, h: HcpIdent) -> &mut CntReg     { self.cnt_regs   .get_mut(*h.get_arena_handle()) }
    pub fn get_cond_wait_reg_mut (&mut self, h: HcpIdent) -> &mut CondWaitStateReg  { self.cond_wait_regs .get_mut(*h.get_arena_handle()) }
    pub fn get_cycle_wait_reg_mut(&mut self, h: HcpIdent) -> &mut CycleWaitStateReg { self.cycle_wait_regs.get_mut(*h.get_arena_handle()) }

    // -----------------------------------------------------------------------
    // HCP trait-object dispatch
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
    // Temp-take / replace-back (for mut self + &mut self re-borrow pattern)
    // -----------------------------------------------------------------------
    pub fn take_reg      (&mut self, h: HcpIdent) -> Reg        { self.regs       .take(*h.get_arena_handle()) }
    pub fn take_wire     (&mut self, h: HcpIdent) -> Wire       { self.wires      .take(*h.get_arena_handle()) }
    pub fn take_val      (&mut self, h: HcpIdent) -> Val        { self.vals       .take(*h.get_arena_handle()) }
    pub fn take_mem_ele  (&mut self, h: HcpIdent) -> MemEle     { self.mem_eles   .take(*h.get_arena_handle()) }
    pub fn take_mem_blk  (&mut self, h: HcpIdent) -> MemBlk     { self.mem_blks   .take(*h.get_arena_handle()) }
    pub fn take_expression(&mut self, h: HcpIdent) -> Expression { self.expressions.take(*h.get_arena_handle()) }
    pub fn take_state_reg(&mut self, h: HcpIdent) -> StateReg   { self.state_regs .take(*h.get_arena_handle()) }
    pub fn take_sync_reg (&mut self, h: HcpIdent) -> SyncReg    { self.sync_regs  .take(*h.get_arena_handle()) }
    pub fn take_cnt_reg  (&mut self, h: HcpIdent) -> CntReg     { self.cnt_regs   .take(*h.get_arena_handle()) }
    pub fn take_cond_wait_reg (&mut self, h: HcpIdent) -> CondWaitStateReg  { self.cond_wait_regs .take(*h.get_arena_handle()) }
    pub fn take_cycle_wait_reg(&mut self, h: HcpIdent) -> CycleWaitStateReg { self.cycle_wait_regs.take(*h.get_arena_handle()) }

    pub fn replace_back_reg      (&mut self, h: HcpIdent, v: Reg)        { self.regs       .replace_back(*h.get_arena_handle(), v) }
    pub fn replace_back_wire     (&mut self, h: HcpIdent, v: Wire)       { self.wires      .replace_back(*h.get_arena_handle(), v) }
    pub fn replace_back_val      (&mut self, h: HcpIdent, v: Val)        { self.vals       .replace_back(*h.get_arena_handle(), v) }
    pub fn replace_back_mem_ele  (&mut self, h: HcpIdent, v: MemEle)     { self.mem_eles   .replace_back(*h.get_arena_handle(), v) }
    pub fn replace_back_mem_blk  (&mut self, h: HcpIdent, v: MemBlk)     { self.mem_blks   .replace_back(*h.get_arena_handle(), v) }
    pub fn replace_back_expression(&mut self, h: HcpIdent, v: Expression) { self.expressions.replace_back(*h.get_arena_handle(), v) }
    pub fn replace_back_state_reg(&mut self, h: HcpIdent, v: StateReg)   { self.state_regs .replace_back(*h.get_arena_handle(), v) }
    pub fn replace_back_sync_reg (&mut self, h: HcpIdent, v: SyncReg)    { self.sync_regs  .replace_back(*h.get_arena_handle(), v) }
    pub fn replace_back_cnt_reg  (&mut self, h: HcpIdent, v: CntReg)     { self.cnt_regs   .replace_back(*h.get_arena_handle(), v) }
    pub fn replace_back_cond_wait_reg (&mut self, h: HcpIdent, v: CondWaitStateReg)  { self.cond_wait_regs .replace_back(*h.get_arena_handle(), v) }
    pub fn replace_back_cycle_wait_reg(&mut self, h: HcpIdent, v: CycleWaitStateReg) { self.cycle_wait_regs.replace_back(*h.get_arena_handle(), v) }

    // -----------------------------------------------------------------------
    // Module insert / get / take / replace_back
    // -----------------------------------------------------------------------
    pub fn add_module(&mut self, m: Module) -> ModuleIdent {
        let h = self.modules.insert(m);
        self.modules.get(h).get_ident()
    }

    pub fn get_module     (&self,     i: ModuleIdent) -> &Module     { self.modules.get    (*i.get_arena_handle()) }
    pub fn get_module_mut (&mut self, i: ModuleIdent) -> &mut Module { self.modules.get_mut(*i.get_arena_handle()) }

    pub fn take_module        (&mut self, i: ModuleIdent)         -> Module { self.modules.take(*i.get_arena_handle()) }
    pub fn replace_back_module(&mut self, i: ModuleIdent, v: Module)        { self.modules.replace_back(*i.get_arena_handle(), v) }
}
