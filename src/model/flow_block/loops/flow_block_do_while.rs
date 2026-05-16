use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::flow_block::common::DoWhileSchematic;
use crate::model::flow_block::flow_block_base::{FlowBlock, FlowBlockBase};
use crate::model::flow_block::flow_block_ident::{FlowBlockIdent, FlowBlockJoinPolicy, FlowBlockType};
use crate::model::flow_block::node_wrap::NodeWrap;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::NcpIdent;

#[derive(Clone, Debug)]
pub struct FlowBlockDoWhile {
    base     : FlowBlockBase,
    schematic: DoWhileSchematic,
    result   : Option<NodeWrap>,
}

impl Default for FlowBlockDoWhile {
    fn default() -> Self { Self::new("", HcpIdent::default()) }
}

impl FlowBlockDoWhile {
    pub fn new(name: &str, cond_i: HcpIdent) -> Self {
        Self {
            base:      FlowBlockBase::new(FlowBlockType::DoWhile, FlowBlockJoinPolicy::SubFlow, name),
            schematic: DoWhileSchematic::new(cond_i),
            result:    None,
        }
    }
}

impl FlowBlock for FlowBlockDoWhile {
    fn get_base    (&self)     -> &FlowBlockBase     { &self.base }
    fn get_base_mut(&mut self) -> &mut FlowBlockBase { &mut self.base }

    fn add_element_in_flow_block(&mut self, _node: NcpIdent) {
        panic!("do-while block does not accept direct asm nodes; use a sub-block")
    }

    fn add_sub_flow_block(&mut self, block: FlowBlockIdent) {
        assert!(self.base.get_sub_blocks_i().is_empty(), "do-while block already has a body sub-block");
        self.base.add_sub_flow_block(block);
    }

    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_flow_block_do_while(*self);
    }

    fn build_hw_component(&mut self, arena: &mut ModelArena) {
        self.result = Some(self.schematic.build(&mut self.base, arena));
    }

    fn summarize_as_block(&self) -> NodeWrap {
        self.result.clone().expect("do-while block has not been built")
    }

}

impl Identifiable for FlowBlockDoWhile {
    fn get_ident_base    (&self)     -> &IdentBase     { self.base.get_ident_ref().get_ident_base() }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.base.get_ident_mut().get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.base.get_ident_mut().build_unique_name() }
}
