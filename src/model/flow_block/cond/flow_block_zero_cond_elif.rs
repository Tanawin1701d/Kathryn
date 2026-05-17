use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::flow_block::flow_block_base::{FlowBlock, FlowBlockBase};
use crate::model::flow_block::flow_block_ident::{FlowBlockIdent, FlowBlockJoinPolicy, FlowBlockType};
use crate::model::flow_block::node_wrap::NodeWrap;
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::NcpIdent;

/// Continuation branch of a zero-cycle conditional chain.
/// `condition: Some(_)` = ZELIF, `condition: None` = ZELSE.
/// Runs only the per-branch GRP pass; the master ZIF reads `grp_asms`
/// to perform the cross-branch merge.
#[derive(Clone, Debug)]
pub struct FlowBlockZeroCondElif {
    base      : FlowBlockBase,
    condition : Option<HcpIdent>,
    grp_asms  : Vec<AssignMeta>,
}

impl Default for FlowBlockZeroCondElif {
    fn default() -> Self { Self::new_zelse("") }
}

impl FlowBlockZeroCondElif {
    pub fn new_zelif(name: &str, cond_i: HcpIdent) -> Self {
        Self {
            base     : FlowBlockBase::new(FlowBlockType::ZeroCondElif, FlowBlockJoinPolicy::ConFlow, name),
            condition: Some(cond_i),
            grp_asms : Vec::new(),
        }
    }

    pub fn new_zelse(name: &str) -> Self {
        Self {
            base     : FlowBlockBase::new(FlowBlockType::ZeroCondElif, FlowBlockJoinPolicy::ConFlow, name),
            condition: None,
            grp_asms : Vec::new(),
        }
    }

    pub fn get_condition(&self) -> Option<HcpIdent> { self.condition }
    pub fn get_grp_asms (&self) -> &[AssignMeta]    { &self.grp_asms }
}

impl FlowBlock for FlowBlockZeroCondElif {
    fn get_base    (&self)     -> &FlowBlockBase     { &self.base }
    fn get_base_mut(&mut self) -> &mut FlowBlockBase { &mut self.base }

    fn add_element_in_flow_block(&mut self, _node: NcpIdent) {
        self.base.add_basic_node(_node);
    }

    fn add_sub_flow_block(&mut self, block: FlowBlockIdent) {
        assert_eq!(block.get_join_policy(), FlowBlockJoinPolicy::BasicNodeFlow,
                   "zero-cond-if sub blocks must have BasicNodeFlow join policy");
        self.base.add_sub_flow_block(block);
    }

    fn add_con_flow_block(&mut self, block: FlowBlockIdent) {
        panic!("zlif not supported con flow blocks");
    }


    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_flow_block_zero_cond_elif(*self);
    }

    fn build_hw_component(&mut self, arena: &mut ModelArena) {
        // Per-branch GRP pass: merge basic-node assign metas within this block.
        self.grp_asms = self.base.gen_unified_asm_meta_flat(arena);
    }
    
    fn get_con_condition(&self) -> Option<HcpIdent> {
        self.condition
    }

    // no block summarization

}

impl Identifiable for FlowBlockZeroCondElif {
    fn get_ident_base    (&self)     -> &IdentBase     { self.base.get_ident_ref().get_ident_base() }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.base.get_ident_mut().get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.base.get_ident_mut().build_unique_name() }
}
