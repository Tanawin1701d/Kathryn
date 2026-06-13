use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::flow_block::flow_block_base::{FlowBlock, FlowBlockBase};
use crate::model::flow_block::flow_block_ident::{FlowBlockIdent, FlowBlockJoinPolicy, FlowBlockType};
use crate::model::flow_block::node_wrap::NodeWrap;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::NcpIdent;

/// Represents a CSELIF (Some(cond)) or CSELSE (None) branch of a cond block.
/// Transparent wrapper: its NodeWrap is exactly its single inner sub-block's NodeWrap.
#[derive(Clone, Debug)]
pub struct FlowBlockCondElif {
    base     : FlowBlockBase,
    condition: Option<HcpIdent>, // None = else
    result   : Option<NodeWrap>,
}

impl Default for FlowBlockCondElif {
    fn default() -> Self { Self::new_else("") }
}

impl FlowBlockCondElif {
    pub fn new_elif(name: &str, cond_i: HcpIdent) -> Self {
        Self {
            base:      FlowBlockBase::new(FlowBlockType::CondElif, FlowBlockJoinPolicy::ConFlow, name, false),
            condition: Some(cond_i),
            result:    None,
        }
    }

    pub fn new_else(name: &str) -> Self {
        Self {
            base:      FlowBlockBase::new(FlowBlockType::CondElif, FlowBlockJoinPolicy::ConFlow, name, false),
            condition: None,
            result:    None,
        }
    }

    pub fn get_con_condition(&self) -> Option<HcpIdent> { self.condition }
}

impl FlowBlock for FlowBlockCondElif {
    fn get_base    (&self)     -> &FlowBlockBase     { &self.base }
    fn get_base_mut(&mut self) -> &mut FlowBlockBase { &mut self.base }

    fn add_element_in_flow_block(&mut self, _node: NcpIdent) {
        panic!("cond elif/else block does not accept direct asm nodes; use a sub-block")
    }

    fn add_sub_flow_block(&mut self, block: FlowBlockIdent) {
        assert!(self.base.get_sub_blocks_i().is_empty(), "cond elif/else already has a body sub-block");
        self.base.add_sub_flow_block(block);
    }

    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_flow_block_cond_elif(*self);
    }

    fn build_hw_component(&mut self, arena: &mut ModelArena) {
        assert!(!self.base.get_sub_blocks_i().is_empty(), "cond elif/else block has no body sub-block");
        // Transparent: proxy the inner sub-block's NodeWrap directly
        self.result = Some(arena.summarize_flow_block(self.base.get_sub_blocks_i()[0]));
    }

    fn check_prefinalize(&self) -> Result<(), String> {
        if self.base.get_sub_blocks_i().is_empty() {
            return Err("cond elif/else block has no body sub-block".to_string());
        }
        Ok(())
    }

    fn summarize_as_block(&self) -> NodeWrap {
        self.result.clone().expect("cond elif/else block has not been built")
    }

}


impl Identifiable for FlowBlockCondElif {
    fn get_ident_base    (&self)     -> &IdentBase     { self.base.get_ident_ref().get_ident_base() }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.base.get_ident_mut().get_ident_base_mut() }
}
