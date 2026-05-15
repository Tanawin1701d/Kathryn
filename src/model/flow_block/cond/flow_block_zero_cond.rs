use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::flow_block::flow_block_base::{FlowBlock, FlowBlockBase};
use crate::model::flow_block::flow_block_ident::{FlowBlockIdent, FlowBlockType};
use crate::model::flow_block::node_wrap::NodeWrap;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::NcpIdent;

/// Zero-cycle conditional block covering ZIF (Some cond, no prior branch),
/// ZELIF (Some cond, chained branch), and ZELSE (None).
/// Hardware build is not implemented here; these are extracted at a higher
/// controller level before any hardware generation occurs.
#[derive(Clone, Debug)]
pub struct FlowBlockZeroCond {
    base           : FlowBlockBase,
    condition      : Option<HcpIdent>,
    master_of_chain: bool,
}

impl Default for FlowBlockZeroCond {
    fn default() -> Self { Self::new_zelse("") }
}

impl FlowBlockZeroCond {
    pub fn new_zif(name: &str, cond_i: HcpIdent) -> Self {
        Self {
            base           :      FlowBlockBase::new(FlowBlockType::ZeroCond, name),
            condition      : Some(cond_i),
            master_of_chain: true,
        }
    }

    pub fn new_zelif(name: &str, cond_i: HcpIdent) -> Self {
        Self {
            base           : FlowBlockBase::new(FlowBlockType::ZeroCond, name),
            condition      : Some(cond_i),
            master_of_chain: false,
        }
    }

    pub fn new_zelse(name: &str) -> Self {
        Self {
            base           : FlowBlockBase::new(FlowBlockType::ZeroCond, name),
            condition      : None,
            master_of_chain: false,
        }
    }
}

impl FlowBlock for FlowBlockZeroCond {
    fn get_base    (&self)     -> &FlowBlockBase     { &self.base }
    fn get_base_mut(&mut self) -> &mut FlowBlockBase { &mut self.base }

    fn add_element_in_flow_block(&mut self, _node: NcpIdent) {
        panic!("zero-cond block does not accept direct asm nodes; use a sub-block")
    }

    fn add_sub_flow_block(&mut self, block: FlowBlockIdent) {
        self.base.add_sub_flow_block(block);
    }

    fn add_con_flow_block(&mut self, block: FlowBlockIdent) {
        self.base.add_con_flow_block(block);
    }

    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_flow_block_zero_cond(*self);
    }

    fn build_hw_component(&mut self, _arena: &mut ModelArena) {
        unreachable!("build_hw_master prevents reaching build_hw_component")
    }

    fn build_hw_master(&mut self, _arena: &mut ModelArena) {
        
    }

    fn summarize_block(&self) -> NodeWrap {
        todo!("ZIF/ZELIF/ZELSE summarize not implemented — requires controller-level extraction before synthesis")
    }

    fn get_con_condition(&self) -> Option<HcpIdent> {
        self.condition
    }
    
    fn get_unified_node(&self) -> NcpIdent{
        
    }
    
}

impl Identifiable for FlowBlockZeroCond {
    fn get_ident_base    (&self)     -> &IdentBase     { self.base.get_ident_ref().get_ident_base() }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.base.get_ident_mut().get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.base.get_ident_mut().build_unique_name() }
}
