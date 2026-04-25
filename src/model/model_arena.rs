use crate::common::arena_base::ArenaGroup;
use crate::model::hw_component::expression::Expression;
use crate::model::hw_component::memBlk::MemBlk;
use crate::model::hw_component::memEle::MemEle;
use crate::model::hw_component::reg::Reg;
use crate::model::hw_component::val::Val;
use crate::model::hw_component::wire::Wire;
use crate::model::hw_component::sp_reg::state_reg::StateReg;

pub struct ModelArena {
    pub(super) regs       : ArenaGroup<Reg>,
    pub(super) wires      : ArenaGroup<Wire>,
    pub(super) vals       : ArenaGroup<Val>,
    pub(super) mem_eles   : ArenaGroup<MemEle>,
    pub(super) mem_blks   : ArenaGroup<MemBlk>,
    pub(super) expressions: ArenaGroup<Expression>,
    pub(super) state_regs : ArenaGroup<StateReg>,
}
