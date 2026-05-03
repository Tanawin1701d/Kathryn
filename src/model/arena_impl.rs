use crate::common::arena_base::{ArenaGroup, ArenaHandle};
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
use crate::model::hw_component::sp_reg::state_reg::StateReg;
use crate::model::model_arena::ModelArena;

macro_rules! dispatch_hcp {
    ($self:expr, $hcpIdent:expr, $method:ident) => {{
        let handle = *$hcpIdent.get_arena_handle();
        match $hcpIdent.get_hw_type() {
            HwComponentType::Reg             => $self.regs       .$method(handle),
            HwComponentType::Wire            => $self.wires      .$method(handle),
            HwComponentType::Val             => $self.vals       .$method(handle),
            HwComponentType::MemBlockIndexer => $self.mem_eles   .$method(handle),
            HwComponentType::Expression      => $self.expressions.$method(handle),
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
            ue_basics  : ArenaGroup::new(),
            ue_grps    : ArenaGroup::new(),
            ue_conds   : ArenaGroup::new(),
            ue_switches: ArenaGroup::new(),
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
        self.ue_basics   = ArenaGroup::new();
        self.ue_grps     = ArenaGroup::new();
        self.ue_conds    = ArenaGroup::new();
        self.ue_switches = ArenaGroup::new();
    }

    // -----------------------------------------------------------------------
    // HCP inserts (return raw ArenaHandle for use by make_* factory methods)
    // -----------------------------------------------------------------------
    pub fn add_reg      (&mut self, r: Reg)       -> ArenaHandle { self.regs       .insert(r) }
    pub fn add_wire     (&mut self, w: Wire)      -> ArenaHandle { self.wires      .insert(w) }
    pub fn add_val      (&mut self, v: Val)       -> ArenaHandle { self.vals       .insert(v) }
    pub fn add_mem_ele  (&mut self, e: MemEle)    -> ArenaHandle { self.mem_eles   .insert(e) }
    pub fn add_mem_blk  (&mut self, b: MemBlk)    -> ArenaHandle { self.mem_blks   .insert(b) }
    pub fn add_expression(&mut self, e: Expression) -> ArenaHandle { self.expressions.insert(e) }
    pub fn add_state_reg(&mut self, s: StateReg)  -> ArenaHandle { self.state_regs .insert(s) }

    // -----------------------------------------------------------------------
    // HCP getters
    // -----------------------------------------------------------------------
    pub fn get_reg      (&self, h: ArenaHandle) -> &Reg        { self.regs       .get(h) }
    pub fn get_wire     (&self, h: ArenaHandle) -> &Wire       { self.wires      .get(h) }
    pub fn get_val      (&self, h: ArenaHandle) -> &Val        { self.vals       .get(h) }
    pub fn get_mem_ele  (&self, h: ArenaHandle) -> &MemEle     { self.mem_eles   .get(h) }
    pub fn get_mem_blk  (&self, h: ArenaHandle) -> &MemBlk     { self.mem_blks   .get(h) }
    pub fn get_expression(&self, h: ArenaHandle) -> &Expression { self.expressions.get(h) }
    pub fn get_state_reg(&self, h: ArenaHandle) -> &StateReg   { self.state_regs .get(h) }

    pub fn get_reg_mut      (&mut self, h: ArenaHandle) -> &mut Reg        { self.regs       .get_mut(h) }
    pub fn get_wire_mut     (&mut self, h: ArenaHandle) -> &mut Wire       { self.wires      .get_mut(h) }
    pub fn get_val_mut      (&mut self, h: ArenaHandle) -> &mut Val        { self.vals       .get_mut(h) }
    pub fn get_mem_ele_mut  (&mut self, h: ArenaHandle) -> &mut MemEle     { self.mem_eles   .get_mut(h) }
    pub fn get_mem_blk_mut  (&mut self, h: ArenaHandle) -> &mut MemBlk     { self.mem_blks   .get_mut(h) }
    pub fn get_expression_mut(&mut self, h: ArenaHandle) -> &mut Expression { self.expressions.get_mut(h) }
    pub fn get_state_reg_mut(&mut self, h: ArenaHandle) -> &mut StateReg   { self.state_regs .get_mut(h) }

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
        self.get_hcp_assign(ident).get_bit_width()
    }

    // -----------------------------------------------------------------------
    // Temp-take / replace-back (for mut self + &mut self re-borrow pattern)
    // -----------------------------------------------------------------------
    pub fn take_reg      (&mut self, h: ArenaHandle) -> Reg        { self.regs       .take(h) }
    pub fn take_wire     (&mut self, h: ArenaHandle) -> Wire       { self.wires      .take(h) }
    pub fn take_val      (&mut self, h: ArenaHandle) -> Val        { self.vals       .take(h) }
    pub fn take_mem_ele  (&mut self, h: ArenaHandle) -> MemEle     { self.mem_eles   .take(h) }
    pub fn take_mem_blk  (&mut self, h: ArenaHandle) -> MemBlk     { self.mem_blks   .take(h) }
    pub fn take_expression(&mut self, h: ArenaHandle) -> Expression { self.expressions.take(h) }
    pub fn take_state_reg(&mut self, h: ArenaHandle) -> StateReg   { self.state_regs .take(h) }

    pub fn replace_back_reg      (&mut self, h: ArenaHandle, v: Reg)        { self.regs       .replace_back(h, v) }
    pub fn replace_back_wire     (&mut self, h: ArenaHandle, v: Wire)       { self.wires      .replace_back(h, v) }
    pub fn replace_back_val      (&mut self, h: ArenaHandle, v: Val)        { self.vals       .replace_back(h, v) }
    pub fn replace_back_mem_ele  (&mut self, h: ArenaHandle, v: MemEle)     { self.mem_eles   .replace_back(h, v) }
    pub fn replace_back_mem_blk  (&mut self, h: ArenaHandle, v: MemBlk)     { self.mem_blks   .replace_back(h, v) }
    pub fn replace_back_expression(&mut self, h: ArenaHandle, v: Expression) { self.expressions.replace_back(h, v) }
    pub fn replace_back_state_reg(&mut self, h: ArenaHandle, v: StateReg)   { self.state_regs .replace_back(h, v) }
}
