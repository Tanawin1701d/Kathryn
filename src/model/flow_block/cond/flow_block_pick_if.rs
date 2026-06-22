use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::flow_block::flow_block_base::{FlowBlock, FlowBlockBase};
use crate::model::flow_block::flow_block_ident::{FlowBlockIdent, FlowBlockJoinPolicy, FlowBlockType};
use crate::model::flow_block::node_wrap::NodeWrap;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::NcpIdent;

/// One branch of a `pick` block: a PIF (Some(cond)) or the optional PIDEF (None).
/// Transparent wrapper — its NodeWrap is exactly its single inner sub-block's NodeWrap.
/// The master `FlowBlockPick` reads `condition` to gate the branch on its raw signal.
#[derive(Clone, Debug)]
pub struct FlowBlockPickIf {
    base     : FlowBlockBase,
    condition: Option<HcpIdent>, // None = pidef default
    result   : Option<NodeWrap>,
}

impl Default for FlowBlockPickIf {
    fn default() -> Self { Self::new_pidef("") }
}

impl FlowBlockPickIf {
    pub fn new_pif(name: &str, cond_i: HcpIdent) -> Self {
        Self {
            base     : FlowBlockBase::new(FlowBlockType::PickIf, FlowBlockJoinPolicy::SubFlow, name, false),
            condition: Some(cond_i),
            result   : None,
        }
    }

    pub fn new_pidef(name: &str) -> Self {
        Self {
            base     : FlowBlockBase::new(FlowBlockType::PickIf, FlowBlockJoinPolicy::SubFlow, name, false),
            condition: None,
            result   : None,
        }
    }

    pub fn get_pick_condition(&self) -> Option<HcpIdent> { self.condition }
}

impl FlowBlock for FlowBlockPickIf {
    fn get_base    (&self)     -> &FlowBlockBase     { &self.base }
    fn get_base_mut(&mut self) -> &mut FlowBlockBase { &mut self.base }

    fn add_element_in_flow_block(&mut self, _node: NcpIdent) {
        panic!("pick branch (pif/pidef) does not accept direct asm nodes; use a sub-block")
    }

    fn add_sub_flow_block(&mut self, block: FlowBlockIdent) {
        assert!(self.base.get_sub_blocks_i().is_empty(), "pick branch already has a body sub-block");
        self.base.add_sub_flow_block(block);
    }

    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_flow_block_pick_if(*self);
    }

    fn build_hw_component(&mut self, arena: &mut ModelArena) {
        assert!(!self.base.get_sub_blocks_i().is_empty(), "pick branch has no body sub-block");
        // Transparent: proxy the inner sub-block's NodeWrap directly
        self.result = Some(arena.summarize_flow_block(self.base.get_sub_blocks_i()[0]));
    }

    fn check_prefinalize(&self) -> Result<(), String> {
        if self.base.get_sub_blocks_i().is_empty() {
            return Err("pick branch (pif/pidef) has no body sub-block".to_string());
        }
        Ok(())
    }

    fn summarize_as_block(&self) -> NodeWrap {
        self.result.clone().expect("pick branch has not been built")
    }
}

impl Identifiable for FlowBlockPickIf {
    fn get_ident_base    (&self)     -> &IdentBase     { self.base.get_ident_ref().get_ident_base() }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.base.get_ident_mut().get_ident_base_mut() }
}
