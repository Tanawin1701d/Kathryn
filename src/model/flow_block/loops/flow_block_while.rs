use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::flow_block::common::{LoopMode, WhileSchematic};
use crate::model::flow_block::flow_block_base::{FlowBlock, FlowBlockBase};
use crate::model::flow_block::flow_block_ident::{FlowBlockIdent, FlowBlockJoinPolicy, FlowBlockType};
use crate::model::flow_block::node_wrap::NodeWrap;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::NcpIdent;

#[derive(Clone, Debug)]
pub struct FlowBlockWhile {
    base     : FlowBlockBase,
    schematic: WhileSchematic,
    result   : Option<NodeWrap>,
}

impl Default for FlowBlockWhile {
    fn default() -> Self { Self::new_cwhile("", HcpIdent::default()) }
}

impl FlowBlockWhile {
    pub fn new_cwhile(name: &str, cond_i: HcpIdent) -> Self {
        Self {
            base:      FlowBlockBase::new(FlowBlockType::WhileLoop, FlowBlockJoinPolicy::SubFlow, name),
            schematic: WhileSchematic::new(LoopMode::Combinatorial, cond_i),
            result:    None,
        }
    }

    pub fn new_swhile(name: &str, cond_i: HcpIdent) -> Self {
        Self {
            base:      FlowBlockBase::new(FlowBlockType::WhileLoop, FlowBlockJoinPolicy::SubFlow, name),
            schematic: WhileSchematic::new(LoopMode::Sequential, cond_i),
            result:    None,
        }
    }
}

impl FlowBlock for FlowBlockWhile {
    fn get_base    (&self)     -> &FlowBlockBase     { &self.base }
    fn get_base_mut(&mut self) -> &mut FlowBlockBase { &mut self.base }

    fn add_element_in_flow_block(&mut self, _node: NcpIdent) {
        panic!("while block does not accept direct asm nodes; use a sub-block")
    }

    fn add_sub_flow_block(&mut self, block: FlowBlockIdent) {
        assert!(self.base.get_sub_blocks_i().is_empty(), "while block already has a body sub-block");
        self.base.add_sub_flow_block(block);
    }

    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_flow_block_while(*self);
    }

    fn build_hw_component(&mut self, arena: &mut ModelArena) {
        self.result = Some(self.schematic.build(&mut self.base, arena));
    }

    fn summarize_as_block(&self) -> NodeWrap {
        self.result.clone().expect("while block has not been built")
    }

}

impl Identifiable for FlowBlockWhile {
    fn get_ident_base    (&self)     -> &IdentBase     { self.base.get_ident_ref().get_ident_base() }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.base.get_ident_mut().get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.base.get_ident_mut().build_unique_name() }
}
