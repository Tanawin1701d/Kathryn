use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::flow_block::common::SeqSchematic;
use crate::model::flow_block::flow_block_base::{FlowBlock, FlowBlockBase};
use crate::model::flow_block::flow_block_ident::{FlowBlockIdent, FlowBlockJoinPolicy, FlowBlockType};
use crate::model::flow_block::node_wrap::NodeWrap;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::NcpIdent;

#[derive(Clone, Debug)]
pub struct FlowBlockSeq {
    base     : FlowBlockBase,
    schematic: SeqSchematic,
    result   : Option<NodeWrap>,
}

impl Default for FlowBlockSeq {
    fn default() -> Self { Self::new("") }
}

impl FlowBlockSeq {
    pub fn new(name: &str) -> Self {
        Self {
            base:      FlowBlockBase::new(FlowBlockType::Sequential, FlowBlockJoinPolicy::SubFlow, name),
            schematic: SeqSchematic::new(),
            result:    None,
        }
    }
}

impl FlowBlock for FlowBlockSeq {
    fn get_base    (&self)     -> &FlowBlockBase     { &self.base }
    fn get_base_mut(&mut self) -> &mut FlowBlockBase { &mut self.base }

    fn add_element_in_flow_block(&mut self, node: NcpIdent) {
        self.schematic.add_node(node);
        self.base.add_basic_node(node);
    }

    fn add_sub_flow_block(&mut self, block: FlowBlockIdent) {
        self.schematic.add_sub_block(block);
        self.base.add_sub_flow_block(block);
    }

    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_flow_block_seq(*self);
    }

    fn build_hw_component(&mut self, arena: &mut ModelArena) {
        self.result = Some(self.schematic.build(&mut self.base, arena));
    }

    fn summarize_as_block(&self) -> NodeWrap {
        self.result.clone().expect("flow block has not been built")
    }

}

impl Identifiable for FlowBlockSeq {
    fn get_ident_base    (&self)     -> &IdentBase     { self.base.get_ident_ref().get_ident_base() }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.base.get_ident_mut().get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.base.get_ident_mut().build_unique_name() }
}
