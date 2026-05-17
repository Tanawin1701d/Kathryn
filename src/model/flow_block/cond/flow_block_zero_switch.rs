use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::flow_block::flow_block_base::{FlowBlock, FlowBlockBase};
use crate::model::flow_block::flow_block_ident::{FlowBlockIdent, FlowBlockJoinPolicy, FlowBlockType};
use crate::model::hw_component::common::asm_meta_helper::AssignMetaSwitchPool;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::NcpIdent;

/// Master zero-cycle switch block.
/// Owns the switch variable (`state_i`) and the first arm's `match_val`.
/// Con blocks are `FlowBlockZeroSwitchCase` arms (each with their own `match_val`).
/// Runs GRP pass on its own basic nodes then merges all arms via `AssignMetaSwitchPool`.
#[derive(Clone, Debug)]
pub struct FlowBlockZeroSwitch {
    base             : FlowBlockBase,
    state_i          : HcpIdent,
    result_basic_node: Option<NcpIdent>,
}

impl Default for FlowBlockZeroSwitch {
    fn default() -> Self { Self::new("", HcpIdent::default()) }
}

impl FlowBlockZeroSwitch {
    pub fn new(name: &str, state_i: HcpIdent) -> Self {
        Self {
            base             : FlowBlockBase::new(FlowBlockType::ZeroSwitch, FlowBlockJoinPolicy::BasicNodeFlow, name),
            state_i,
            result_basic_node: None,
        }
    }
}

impl FlowBlock for FlowBlockZeroSwitch {
    fn get_base    (&self)     -> &FlowBlockBase     { &self.base }
    fn get_base_mut(&mut self) -> &mut FlowBlockBase { &mut self.base }

    fn add_element_in_flow_block(&mut self, _node: NcpIdent) {
        panic!("FlowBlockZeroSwitch add_element_in_flow_block shouldn't be called!");
    }

    fn add_sub_flow_block(&mut self, block: FlowBlockIdent) {
        assert_eq!(block.get_block_type(), FlowBlockType::ZeroSwitchCase,
            "zero-switch sub blocks must be ZeroSwitchCase");
        self.base.add_sub_flow_block(block);
    }

    fn add_con_flow_block(&mut self, _block: FlowBlockIdent) {
        panic!("FlowBlockZeroSwitch add_con_flow_block shouldn't be called!");
    }

    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_flow_block_zero_switch(*self);
    }

    fn build_hw_component(&mut self, arena: &mut ModelArena) {
        // Cross-arm merge via AssignMetaSwitchPool.
        let id      = self.base.get_ident().get_global_id();
        let state_i = self.state_i;
        let mut pool = AssignMetaSwitchPool::default();

        let case_blocks_i: Vec<FlowBlockIdent> = self.base.get_sub_blocks_i().to_vec();
        for case_block_i in case_blocks_i {
            let case = arena.take_flow_block_zero_switch_case(case_block_i);
            pool.insert_asms(arena, state_i, case.get_match_val(), &case.get_grp_asms().to_vec());
            arena.replace_back_flow_block_zero_switch_case(case);
        }

        // Finalise: flatten all arms into a single asm node.
        let result_asms = pool.gen_assign_metas(arena);
        self.result_basic_node = Some(arena.make_asm_node_many(
            &format!("zsw_result_{}", id),
            &result_asms,
        ));
    }

    /// block summarization
    fn summarize_as_node(&self) -> NcpIdent { self.result_basic_node.expect("zero-switch has not been built yet") }
}

impl Identifiable for FlowBlockZeroSwitch {
    fn get_ident_base    (&self)     -> &IdentBase     { self.base.get_ident_ref().get_ident_base() }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.base.get_ident_mut().get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.base.get_ident_mut().build_unique_name() }
}
