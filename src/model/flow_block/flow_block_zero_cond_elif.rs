use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::flow_block::flow_block_base::{FlowBlock, FlowBlockBase};
use crate::model::flow_block::flow_block_ident::{FlowBlockIdent, FlowBlockType};
use crate::model::flow_block::node_wrap::NodeWrap;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::NcpIdent;

/// ZELIF (Some(cond)) or ZELSE (None) — zero-cycle elif/else branch (stub).
/// Hardware build is not implemented here; these are extracted at a higher
/// controller level before any hardware generation occurs.
#[derive(Clone, Debug)]
pub struct FlowBlockZeroCondElif {
    base     : FlowBlockBase,
    condition: Option<HcpIdent>, // None = zelse
}

impl Default for FlowBlockZeroCondElif {
    fn default() -> Self { Self::new_zelse("") }
}

impl FlowBlockZeroCondElif {
    pub fn new_zelif(name: &str, cond_i: HcpIdent) -> Self {
        Self {
            base:      FlowBlockBase::new(FlowBlockType::ZeroCondElif, name),
            condition: Some(cond_i),
        }
    }

    pub fn new_zelse(name: &str) -> Self {
        Self {
            base:      FlowBlockBase::new(FlowBlockType::ZeroCondElif, name),
            condition: None,
        }
    }
}

impl FlowBlock for FlowBlockZeroCondElif {
    fn get_base    (&self)     -> &FlowBlockBase     { &self.base }
    fn get_base_mut(&mut self) -> &mut FlowBlockBase { &mut self.base }

    fn add_element_in_flow_block(&mut self, _node: NcpIdent) {
        panic!("zero-cond elif/else block does not accept direct asm nodes; use a sub-block")
    }

    fn add_sub_flow_block(&mut self, block: FlowBlockIdent) {
        self.base.add_sub_flow_block(block);
    }

    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_flow_block_zero_cond_elif(*self);
    }

    fn build_hw_master(&mut self, _arena: &mut ModelArena) {
        todo!("ZELIF/ZELSE hardware build not implemented — requires controller-level extraction before synthesis")
    }

    fn build_hw_component(&mut self, _arena: &mut ModelArena) {
        unreachable!("build_hw_master prevents reaching build_hw_component")
    }

    fn summarize_block(&self) -> NodeWrap {
        todo!("ZELIF/ZELSE summarize not implemented — requires controller-level extraction before synthesis")
    }

    fn get_con_condition(&self) -> Option<HcpIdent> {
        self.condition
    }
}

impl Identifiable for FlowBlockZeroCondElif {
    fn get_ident_base    (&self)     -> &IdentBase     { self.base.get_ident_ref().get_ident_base() }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.base.get_ident_mut().get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.base.get_ident_mut().build_unique_name() }
}
