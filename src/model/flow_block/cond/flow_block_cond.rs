use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::flow_block::common::{CondMode, CondSchematic};
use crate::model::flow_block::flow_block_base::{FlowBlock, FlowBlockBase};
use crate::model::flow_block::flow_block_ident::{FlowBlockIdent, FlowBlockJoinPolicy, FlowBlockType};
use crate::model::flow_block::node_wrap::NodeWrap;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::NcpIdent;

#[derive(Clone, Debug)]
pub struct FlowBlockCond {
    base     : FlowBlockBase,
    schematic: CondSchematic,
    result   : Option<NodeWrap>,
}

impl Default for FlowBlockCond {
    fn default() -> Self { Self::new_cif("", HcpIdent::default()) }
}

impl FlowBlockCond {
    pub fn new_cif(name: &str, cond_i: HcpIdent) -> Self {
        Self {
            base:      FlowBlockBase::new(FlowBlockType::CondIf, FlowBlockJoinPolicy::SubFlow, name),
            schematic: CondSchematic::new(CondMode::Combinatorial, cond_i),
            result:    None,
        }
    }

    pub fn new_sif(name: &str, cond_i: HcpIdent) -> Self {
        Self {
            base:      FlowBlockBase::new(FlowBlockType::CondIf, FlowBlockJoinPolicy::SubFlow, name),
            schematic: CondSchematic::new(CondMode::Sequential, cond_i),
            result:    None,
        }
    }
}

impl FlowBlock for FlowBlockCond {
    fn get_base    (&self)     -> &FlowBlockBase     { &self.base }
    fn get_base_mut(&mut self) -> &mut FlowBlockBase { &mut self.base }

    fn add_element_in_flow_block(&mut self, _node: NcpIdent) {
        panic!("cond block does not accept direct asm nodes; use a sub-block")
    }

    fn add_sub_flow_block(&mut self, block: FlowBlockIdent) {
        assert!(self.base.get_sub_blocks_i().is_empty(), "cond block already has a body sub-block");
        self.base.add_sub_flow_block(block);
    }

    fn add_con_flow_block(&mut self, block: FlowBlockIdent) {
        self.base.add_con_flow_block(block);
    }

    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_flow_block_cond(*self);
    }

    fn build_hw_component(&mut self, arena: &mut ModelArena) {
        self.result = Some(self.schematic.build(&mut self.base, arena));
    }

    fn summarize_block(&self) -> NodeWrap {
        self.result.clone().expect("cond block has not been built")
    }

}

impl Identifiable for FlowBlockCond {
    fn get_ident_base    (&self)     -> &IdentBase     { self.base.get_ident_ref().get_ident_base() }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.base.get_ident_mut().get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.base.get_ident_mut().build_unique_name() }
}
