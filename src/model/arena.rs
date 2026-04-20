use crate::common::arena_base::{ArenaGroup, ArenaHandle};
use crate::model::common::identifier::Identifiable;
use crate::model::hw_component::common::hcp_assign::HcpAssignable;
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HwComponentType};
use crate::model::hw_component::common::hcp_read::HcpReadable;
use crate::model::hw_component::memBlk::MemBlk;
use crate::model::hw_component::memEle::MemEle;
use crate::model::hw_component::reg::Reg;
use crate::model::hw_component::val::Val;
use crate::model::hw_component::wire::Wire;

macro_rules! dispatch_hcp {
    ($self:expr, $ident:expr, $method:ident) => {{
        // Get the arena handle from the identifier
        let handle = *$ident.get_arena_handle();
        
        // Match on component type and call method on correct arena
        match $ident.get_hw_type() {
            HwComponentType::Reg             => $self.regs    .$method(handle),
            HwComponentType::Wire            => $self.wires   .$method(handle),
            HwComponentType::Val             => $self.vals    .$method(handle),
            HwComponentType::MemBlockIndexer => $self.mem_eles.$method(handle),
            t => panic!("HwComponentType {:?} is not HCP-accessible", t),
        }
    }};
}

pub struct Arena {
    regs     : ArenaGroup<Reg>,
    wires    : ArenaGroup<Wire>,
    vals     : ArenaGroup<Val>,
    mem_eles : ArenaGroup<MemEle>,
    mem_blks : ArenaGroup<MemBlk>,
}

impl Arena {
    pub fn new() -> Self {
        Self {
            regs     : ArenaGroup::new(),
            wires    : ArenaGroup::new(),
            vals     : ArenaGroup::new(),
            mem_eles : ArenaGroup::new(),
            mem_blks : ArenaGroup::new(),
        }
    }

    pub fn add_reg    (&mut self, r: Reg)    -> ArenaHandle {
        self.regs    .insert(r)
    }
    pub fn add_wire   (&mut self, w: Wire)   -> ArenaHandle { self.wires   .insert(w) }
    pub fn add_val    (&mut self, v: Val)     -> ArenaHandle { self.vals    .insert(v) }
    pub fn add_mem_ele(&mut self, e: MemEle) -> ArenaHandle { self.mem_eles.insert(e) }
    pub fn add_mem_blk(&mut self, b: MemBlk) -> ArenaHandle { self.mem_blks.insert(b) }

    pub fn get_reg    (&self, h: ArenaHandle) -> &Reg     { self.regs    .get(h) }
    pub fn get_wire   (&self, h: ArenaHandle) -> &Wire    { self.wires   .get(h) }
    pub fn get_val    (&self, h: ArenaHandle) -> &Val      { self.vals   .get(h) }
    pub fn get_mem_ele(&self, h: ArenaHandle) -> &MemEle  { self.mem_eles.get(h) }
    pub fn get_mem_blk(&self, h: ArenaHandle) -> &MemBlk  { self.mem_blks.get(h) }

    pub fn get_reg_mut    (&mut self, h: ArenaHandle) -> &mut Reg     { self.regs    .get_mut(h) }
    pub fn get_wire_mut   (&mut self, h: ArenaHandle) -> &mut Wire    { self.wires   .get_mut(h) }
    pub fn get_val_mut    (&mut self, h: ArenaHandle) -> &mut Val      { self.vals    .get_mut(h) }
    pub fn get_mem_ele_mut(&mut self, h: ArenaHandle) -> &mut MemEle  { self.mem_eles.get_mut(h) }
    pub fn get_mem_blk_mut(&mut self, h: ArenaHandle) -> &mut MemBlk  { self.mem_blks.get_mut(h) }

    pub fn get_hcp_assign    (&    self, ident: &HcpIdent) -> &    dyn HcpAssignable { dispatch_hcp!(self, ident, get    ) }
    pub fn get_hcp_assign_mut(&mut self, ident: &HcpIdent) -> &mut dyn HcpAssignable { dispatch_hcp!(self, ident, get_mut) }
    pub fn get_hcp_readable    (&    self, ident: &HcpIdent) -> &    dyn HcpReadable { dispatch_hcp!(self, ident, get    ) }
    pub fn get_hcp_readable_mut(&mut self, ident: &HcpIdent) -> &mut dyn HcpReadable { dispatch_hcp!(self, ident, get_mut) }

}
