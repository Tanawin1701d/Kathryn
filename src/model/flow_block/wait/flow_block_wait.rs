use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::flow_block::common::WaitSchematic;
use crate::model::flow_block::flow_block_base::{FlowBlock, FlowBlockBase};
use crate::model::flow_block::flow_block_ident::{FlowBlockIdent, FlowBlockJoinPolicy, FlowBlockType};
use crate::model::flow_block::node_wrap::NodeWrap;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::slice::Slice;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::NcpIdent;

// Leaf flow block that stalls the flow until a wait node releases it. `scwait`
// (cond wait) and `sywait` (cycle wait) share this one struct — the cond/cycle
// choice lives only in `WaitSchematic`, mirroring how FlowBlockPar unifies its
// sync modes.
#[derive(Clone, Debug)]
pub struct FlowBlockWait {
    base     : FlowBlockBase,
    schematic: WaitSchematic,
    result   : Option<NodeWrap>,
}

impl Default for FlowBlockWait {
    fn default() -> Self { Self::new_cycle_wait("", 1) }
}

impl FlowBlockWait {
    pub fn new_cond_wait(name: &str, cond_i: HcpIdent, cond_sl: Slice) -> Self {
        Self {
            base     : FlowBlockBase::new(FlowBlockType::Wait, FlowBlockJoinPolicy::SubFlow, name, false),
            schematic: WaitSchematic::new_cond(cond_i, cond_sl),
            result   : None,
        }
    }

    pub fn new_cycle_wait(name: &str, cycle: i32) -> Self {
        Self {
            base     : FlowBlockBase::new(FlowBlockType::Wait, FlowBlockJoinPolicy::SubFlow, name, false),
            schematic: WaitSchematic::new_cycle(cycle),
            result   : None,
        }
    }
}

impl FlowBlock for FlowBlockWait {
    fn get_base    (&self)     -> &FlowBlockBase     { &self.base }
    fn get_base_mut(&mut self) -> &mut FlowBlockBase { &mut self.base }

    fn add_element_in_flow_block(&mut self, _node: NcpIdent) {
        panic!("wait block accepts no direct nodes")
    }

    fn add_sub_flow_block(&mut self, _block: FlowBlockIdent) {
        panic!("wait block accepts no sub-blocks")
    }

    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_flow_block_wait(*self);
    }

    fn build_hw_component(&mut self, arena: &mut ModelArena) {
        self.result = Some(self.schematic.build(&mut self.base, arena));
    }

    fn check_prefinalize(&self) -> Result<(), String> {
        if !self.base.get_sub_blocks_i().is_empty() {
            return Err("wait block accepts no sub-blocks".to_string());
        }
        if !self.base.get_con_blocks_i().is_empty() {
            return Err("wait block does not support con blocks".to_string());
        }
        Ok(())
    }

    fn summarize_as_block(&self) -> NodeWrap {
        self.result.clone().expect("wait block has not been built")
    }
}

impl Identifiable for FlowBlockWait {
    fn get_ident_base    (&self)     -> &IdentBase     { self.base.get_ident_ref().get_ident_base() }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.base.get_ident_mut().get_ident_base_mut() }
}
