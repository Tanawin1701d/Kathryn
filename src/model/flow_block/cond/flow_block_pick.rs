use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::flow_block::common::PickSchematic;
use crate::model::flow_block::flow_block_base::{FlowBlock, FlowBlockBase};
use crate::model::flow_block::flow_block_ident::{FlowBlockIdent, FlowBlockJoinPolicy, FlowBlockType};
use crate::model::flow_block::node_wrap::NodeWrap;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::NcpIdent;

/// Master `pick` block. Holds no condition itself — each branch is a
/// `FlowBlockPickIf` (pif/pidef) sub-block carrying its own raw condition.
/// Unlike `cif`, branches are NOT mutually-exclusion chained, and the exit is a
/// plain OR over the branch exits — it is NOT auto-synchronized (the branch that
/// matches drives the exit signal).
#[derive(Clone, Debug)]
pub struct FlowBlockPick {
    base     : FlowBlockBase,
    schematic: PickSchematic,
    result   : Option<NodeWrap>,
}

impl Default for FlowBlockPick {
    fn default() -> Self { Self::new("") }
}

impl FlowBlockPick {
    pub fn new(name: &str) -> Self {
        Self {
            base:      FlowBlockBase::new(FlowBlockType::Pick, FlowBlockJoinPolicy::SubFlow, name, false),
            schematic: PickSchematic::new(),
            result:    None,
        }
    }
}

impl FlowBlock for FlowBlockPick {
    fn get_base    (&self)     -> &FlowBlockBase     { &self.base }
    fn get_base_mut(&mut self) -> &mut FlowBlockBase { &mut self.base }

    fn add_element_in_flow_block(&mut self, _node: NcpIdent) {
        panic!("pick block does not accept direct asm nodes; use a pif/pidef branch")
    }

    fn add_sub_flow_block(&mut self, block: FlowBlockIdent) {
        assert_eq!(block.get_block_type(), FlowBlockType::PickIf,
            "pick sub blocks must be pif/pidef (PickIf)");
        self.base.add_sub_flow_block(block);
    }

    fn add_con_flow_block(&mut self, _block: FlowBlockIdent) {
        panic!("pick block does not accept continuation blocks");
    }

    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_flow_block_pick(*self);
    }

    fn build_hw_component(&mut self, arena: &mut ModelArena) {
        eprintln!(
            "[kathryn][warn] pick block '{}': exit is NOT auto-synchronized; \
             the matching branch drives the exit signal",
            self.base.get_ident().get_rel_name(),
        );
        self.result = Some(self.schematic.build(&mut self.base, arena));
    }

    fn check_prefinalize(&self) -> Result<(), String> {
        if self.base.get_sub_blocks_i().is_empty() {
            return Err("pick block must have at least one pif branch".to_string());
        }
        Ok(())
    }

    fn summarize_as_block(&self) -> NodeWrap {
        self.result.clone().expect("pick block has not been built")
    }
}

impl Identifiable for FlowBlockPick {
    fn get_ident_base    (&self)     -> &IdentBase     { self.base.get_ident_ref().get_ident_base() }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.base.get_ident_mut().get_ident_base_mut() }
}
