use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::flow_block::flow_block_base::{FlowBlock, FlowBlockBase};
use crate::model::flow_block::flow_block_ident::{FlowBlockIdent, FlowBlockJoinPolicy, FlowBlockType};
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::NcpIdent;

/// One arm of a zero-cycle switch block.
/// Runs the per-branch GRP pass; the master `FlowBlockZeroSwitch` reads
/// `grp_asms` and `match_val` to perform the cross-arm merge.
#[derive(Clone, Debug)]
pub struct FlowBlockZeroSwitchCase {
    base     : FlowBlockBase,
    match_val: i32,
    grp_asms : Vec<AssignMeta>,
}

impl Default for FlowBlockZeroSwitchCase {
    fn default() -> Self { Self::new("", 0) }
}

impl FlowBlockZeroSwitchCase {
    pub fn new(name: &str, match_val: i32) -> Self {
        Self {
            base     : FlowBlockBase::new(FlowBlockType::ZeroSwitchCase, FlowBlockJoinPolicy::SubFlow, name),
            match_val,
            grp_asms : Vec::new(),
        }
    }

    pub fn get_match_val(&self) -> i32        { self.match_val }
    pub fn get_grp_asms (&self) -> &[AssignMeta] { &self.grp_asms }
}

impl FlowBlock for FlowBlockZeroSwitchCase {
    fn get_base    (&self)     -> &FlowBlockBase     { &self.base }
    fn get_base_mut(&mut self) -> &mut FlowBlockBase { &mut self.base }

    fn add_element_in_flow_block(&mut self, _node: NcpIdent) {
        self.base.add_basic_node(_node);
    }

    fn add_sub_flow_block(&mut self, block: FlowBlockIdent) {
        assert_eq!(block.get_join_policy(), FlowBlockJoinPolicy::BasicNodeFlow,
            "zero-switch-case sub blocks must have BasicNodeFlow join policy");
        self.base.add_sub_flow_block(block);
    }

    fn add_con_flow_block(&mut self, _block: FlowBlockIdent) {
        panic!("zero-switch-case does not accept continuation blocks");
    }

    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_flow_block_zero_switch_case(*self);
    }

    fn build_hw_component(&mut self, arena: &mut ModelArena) {
        self.grp_asms = self.base.gen_unified_asm_meta_flat(arena);
    }

    // no block summarization
}

impl Identifiable for FlowBlockZeroSwitchCase {
    fn get_ident_base    (&self)     -> &IdentBase     { self.base.get_ident_ref().get_ident_base() }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.base.get_ident_mut().get_ident_base_mut() }
}
