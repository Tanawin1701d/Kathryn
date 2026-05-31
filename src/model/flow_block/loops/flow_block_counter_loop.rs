use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::flow_block::common::CounterLoopSchematic;
use crate::model::flow_block::flow_block_base::{FlowBlock, FlowBlockBase};
use crate::model::flow_block::flow_block_ident::{FlowBlockIdent, FlowBlockJoinPolicy, FlowBlockType};
use crate::model::flow_block::node_wrap::NodeWrap;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::NcpIdent;

#[derive(Clone, Debug)]
pub struct FlowBlockCounterLoop {
    base     : FlowBlockBase,
    schematic: CounterLoopSchematic,
    result   : Option<NodeWrap>,
}

impl Default for FlowBlockCounterLoop {
    fn default() -> Self { Self::new("", 1) }
}

impl FlowBlockCounterLoop {
    pub fn new(name: &str, last_loop_cnt: i32) -> Self {
        Self {
            base     : FlowBlockBase::new(FlowBlockType::CounterLoop, FlowBlockJoinPolicy::SubFlow, name),
            schematic: CounterLoopSchematic::new(last_loop_cnt),
            result   :    None,
        }
    }
}

impl FlowBlock for FlowBlockCounterLoop {
    fn get_base    (&self)     -> &FlowBlockBase     { &self.base }
    fn get_base_mut(&mut self) -> &mut FlowBlockBase { &mut self.base }

    fn add_element_in_flow_block(&mut self, _node: NcpIdent) {
        panic!("counter loop does not accept direct asm nodes; use a sub-block")
    }

    fn add_sub_flow_block(&mut self, block: FlowBlockIdent) {
        assert!(self.base.get_sub_blocks_i().is_empty(), "counter loop already has a body sub-block");
        self.base.add_sub_flow_block(block);
    }

    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_flow_block_counter_loop(*self);
    }

    fn build_hw_component(&mut self, arena: &mut ModelArena) {
        self.result = Some(self.schematic.build(&mut self.base, arena));
    }

    fn summarize_as_block(&self) -> NodeWrap {
        self.result.clone().expect("counter loop has not been built")
    }

    fn get_loop_id_expr_i(&self) -> HcpIdent {
        self.schematic.get_loop_id_expr_i()
    }

}

impl Identifiable for FlowBlockCounterLoop {
    fn get_ident_base    (&self)     -> &IdentBase     { self.base.get_ident_ref().get_ident_base() }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.base.get_ident_mut().get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.base.get_ident_mut().build_unique_name() }
}
