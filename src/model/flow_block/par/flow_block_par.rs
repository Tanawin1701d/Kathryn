use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::flow_block::common::{ParSchematic, ParSyncMode};
use crate::model::flow_block::flow_block_base::{FlowBlock, FlowBlockBase};
use crate::model::flow_block::flow_block_ident::{FlowBlockIdent, FlowBlockJoinPolicy, FlowBlockType};
use crate::model::flow_block::node_wrap::NodeWrap;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::NcpIdent;

#[derive(Clone, Debug)]
pub struct FlowBlockPar {
    base     : FlowBlockBase,
    schematic: ParSchematic,
    result   : Option<NodeWrap>,
}

impl Default for FlowBlockPar {
    fn default() -> Self { Self::new_auto_sync("") }
}

impl FlowBlockPar {
    pub fn new_auto_sync(name: &str) -> Self {
        Self {
            base:      FlowBlockBase::new(FlowBlockType::Parallel, FlowBlockJoinPolicy::SubFlow, name, false),
            schematic: ParSchematic::new(ParSyncMode::AutoSync),
            result:    None,
        }
    }

    pub fn new_no_sync(name: &str) -> Self {
        Self {
            base:      FlowBlockBase::new(FlowBlockType::Parallel, FlowBlockJoinPolicy::SubFlow, name, false),
            schematic: ParSchematic::new(ParSyncMode::NoSync),
            result:    None,
        }
    }
}

impl FlowBlock for FlowBlockPar {
    fn get_base    (&self)     -> &FlowBlockBase     { &self.base }
    fn get_base_mut(&mut self) -> &mut FlowBlockBase { &mut self.base }

    fn add_element_in_flow_block(&mut self, node: NcpIdent) {
        self.base.add_basic_node(node);
    }

    fn add_sub_flow_block(&mut self, block: FlowBlockIdent) {
        self.base.add_sub_flow_block(block);
    }

    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_flow_block_par(*self);
    }

    fn build_hw_component(&mut self, arena: &mut ModelArena) {
        self.result = Some(self.schematic.build(&mut self.base, arena));
    }

    fn check_prefinalize(&self) -> Result<(), String> {
        if self.base.get_basic_nodes_i().is_empty() && self.base.get_sub_blocks_i().is_empty() {
            return Err("parallel flow block has no element".to_string());
        }
        if !self.base.get_con_blocks_i().is_empty() {
            return Err("parallel flow block does not support con blocks".to_string());
        }
        Ok(())
    }

    fn summarize_as_block(&self) -> NodeWrap {
        self.result.clone().expect("flow block has not been built")
    }

}

impl Identifiable for FlowBlockPar {
    fn get_ident_base    (&self)     -> &IdentBase     { self.base.get_ident_ref().get_ident_base() }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.base.get_ident_mut().get_ident_base_mut() }
}
