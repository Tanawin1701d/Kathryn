use crate::common::arena_base::ArenaGroup;
use crate::model::hw_component::common::update_event::{UeBasic, UeCond, UeGrp, UeSwitch};
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
use crate::model::nodes::node::NcpNodeEntry;

pub struct ModelArena {

    // basic hardware components
    pub(super) regs       : ArenaGroup<Reg>,
    pub(super) wires      : ArenaGroup<Wire>,
    pub(super) vals       : ArenaGroup<Val>,
    pub(super) mem_eles   : ArenaGroup<MemEle>,
    pub(super) mem_blks   : ArenaGroup<MemBlk>,
    pub(super) expressions: ArenaGroup<Expression>,
    pub(super) state_regs : ArenaGroup<StateReg>,
    pub(super) sync_regs  : ArenaGroup<SyncReg>,
    pub(super) cnt_regs   : ArenaGroup<CntReg>,
    pub(super) cond_wait_regs : ArenaGroup<CondWaitStateReg>,
    pub(super) cycle_wait_regs: ArenaGroup<CycleWaitStateReg>,

    // node components
    pub(super) nodes      : ArenaGroup<NcpNodeEntry>,

    // basic update-event components
    pub(super) ue_basics  : ArenaGroup<UeBasic>,
    pub(super) ue_grps    : ArenaGroup<UeGrp>,
    pub(super) ue_conds   : ArenaGroup<UeCond>,
    pub(super) ue_switches: ArenaGroup<UeSwitch>,
}
