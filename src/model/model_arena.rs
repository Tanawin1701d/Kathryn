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
use crate::model::nodes::asm_node::AsmNode;
use crate::model::nodes::cnt_node::CounterNode;
use crate::model::nodes::logic_node::{OprNode, PseudoNode};
use crate::model::nodes::start_node::StartNode;
use crate::model::nodes::state_node::StateNode;
use crate::model::nodes::syn_node::SynNode;
use crate::model::nodes::wait_node::{WaitCondNode, WaitCycleNode};
use crate::model::module::module::Module;
use crate::model::flow_block::par::flow_block_par::FlowBlockPar;
use crate::model::flow_block::seq::flow_block_seq::FlowBlockSeq;
use crate::model::flow_block::cond::flow_block_cond::FlowBlockCond;
use crate::model::flow_block::cond::flow_block_cond_elif::FlowBlockCondElif;
use crate::model::flow_block::cond::flow_block_zero_cond_if::FlowBlockZeroCondIf;
use crate::model::flow_block::cond::flow_block_zero_cond_elif::FlowBlockZeroCondElif;
use crate::model::flow_block::cond::flow_block_zero_switch::FlowBlockZeroSwitch;
use crate::model::flow_block::cond::flow_block_zero_switch_case::FlowBlockZeroSwitchCase;
use crate::model::flow_block::loops::flow_block_while::FlowBlockWhile;
use crate::model::flow_block::loops::flow_block_do_while::FlowBlockDoWhile;
use crate::model::flow_block::loops::flow_block_counter_loop::FlowBlockCounterLoop;

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

    // basic update-event components
    pub(super) ue_basics  : ArenaGroup<UeBasic>,
    pub(super) ue_grps    : ArenaGroup<UeGrp>,
    pub(super) ue_conds   : ArenaGroup<UeCond>,
    pub(super) ue_switches: ArenaGroup<UeSwitch>,

    // node arenas
    pub(super) asm_nodes        : ArenaGroup<AsmNode>,
    pub(super) start_nodes      : ArenaGroup<StartNode>,
    pub(super) state_nodes      : ArenaGroup<StateNode>,
    pub(super) syn_nodes        : ArenaGroup<SynNode>,
    pub(super) wait_cond_nodes  : ArenaGroup<WaitCondNode>,
    pub(super) wait_cycle_nodes : ArenaGroup<WaitCycleNode>,
    pub(super) counter_nodes    : ArenaGroup<CounterNode>,
    pub(super) pseudo_nodes     : ArenaGroup<PseudoNode>,
    pub(super) opr_nodes        : ArenaGroup<OprNode>,

    // module arena
    pub(super) modules          : ArenaGroup<Module>,

    // flow-block arenas
    pub(super) flow_block_seqs           : ArenaGroup<FlowBlockSeq>,
    pub(super) flow_block_pars           : ArenaGroup<FlowBlockPar>,
    pub(super) flow_block_conds          : ArenaGroup<FlowBlockCond>,
    pub(super) flow_block_cond_elifs     : ArenaGroup<FlowBlockCondElif>,
    pub(super) flow_block_zero_cond_ifs   : ArenaGroup<FlowBlockZeroCondIf>,
    pub(super) flow_block_zero_cond_elifs : ArenaGroup<FlowBlockZeroCondElif>,
    pub(super) flow_block_zero_switches   : ArenaGroup<FlowBlockZeroSwitch>,
    pub(super) flow_block_zero_switch_cases: ArenaGroup<FlowBlockZeroSwitchCase>,
    pub(super) flow_block_whiles         : ArenaGroup<FlowBlockWhile>,
    pub(super) flow_block_do_whiles      : ArenaGroup<FlowBlockDoWhile>,
    pub(super) flow_block_counter_loops  : ArenaGroup<FlowBlockCounterLoop>,
}
