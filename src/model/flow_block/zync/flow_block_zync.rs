use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::complex_hardware::common::ccp_ident::{CcpIdent, CcpType};
use crate::model::flow_block::common::{ZyncArbBind, ZyncSchematic, ZyncSyncMode};
use crate::model::flow_block::flow_block_base::{FlowBlock, FlowBlockBase};
use crate::model::flow_block::flow_block_ident::{FlowBlockIdent, FlowBlockJoinPolicy, FlowBlockType};
use crate::model::flow_block::node_wrap::NodeWrap;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::NcpIdent;

// Zync ("synchronise") flow block: contends on one channel of an arbiter (Arb
// CCP).  Its work nodes (any number of direct asm nodes and/or basic-flow
// sub-blocks) all fire together the cycle the arbiter grants this channel's ACK.
// The arb request-channel (leaf) is allocated up front, when the block is
// created, so the channel index is fixed for the block's whole lifetime.
// All node wiring lives in ZyncSchematic.
#[derive(Clone, Debug)]
pub struct FlowBlockZync {
    base     : FlowBlockBase,
    schematic: ZyncSchematic,
    result   : Option<NodeWrap>,
}

impl Default for FlowBlockZync {
    // Placeholder for arena take/replace_back only; never contends (channel 0 on a
    // dummy arb).  Real blocks come from `new`, whose binds carry allocated leaves.
    fn default() -> Self {
        let placeholder_i = ZyncArbBind::new(CcpIdent::new(CcpType::Arb, false, ""), 0, None);
        Self {
            base     : FlowBlockBase::new(FlowBlockType::Zync, FlowBlockJoinPolicy::SubFlow, "", false),
            schematic: ZyncSchematic::new(vec![placeholder_i], ZyncSyncMode::Any),
            result   : None,
        }
    }
}

impl FlowBlockZync {
    // Contend on every bind, combining grants per `mode`.  Leaves are allocated by
    // the `make_flow_block_zync*` factories; their indices ride inside each bind.
    pub fn new(name: &str, binds: Vec<ZyncArbBind>, mode: ZyncSyncMode) -> Self {
        Self {
            base     : FlowBlockBase::new(FlowBlockType::Zync, FlowBlockJoinPolicy::SubFlow, name, false),
            schematic: ZyncSchematic::new(binds, mode),
            result   : None,
        }
    }
}

impl FlowBlock for FlowBlockZync {
    fn get_base    (&self)     -> &FlowBlockBase     { &self.base }
    fn get_base_mut(&mut self) -> &mut FlowBlockBase { &mut self.base }

    fn add_element_in_flow_block(&mut self, node: NcpIdent) {
        self.base.add_basic_node(node);
    }

    fn add_sub_flow_block(&mut self, block: FlowBlockIdent) {
        assert_eq!(block.get_join_policy(), FlowBlockJoinPolicy::BasicNodeFlow,
                   "zync block accepts only basic-flow sub-blocks; use a direct asm node or a basic-flow block");
        self.base.add_sub_flow_block(block);
    }

    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_flow_block_zync(*self);
    }

    fn build_hw_component(&mut self, arena: &mut ModelArena) {
        // In All ("for all") mode the grant is AND over the binds' (ack & cond): a
        // bind whose condition is false contributes 0, so the work node can fire on
        // a partial set of the intended arbs.  Warn that the target may activate
        // while not every condition is satisfied — but only for a genuine multi-arb
        // block (with one bind All and Some coincide, so there is nothing to warn).
        if self.schematic.get_mode() == ZyncSyncMode::All && self.schematic.get_binds().len() > 1 {
            eprintln!(
                "[kathryn][warn] zync block '{}': All-mode grant is AND over (ack & cond); \
                 the target may activate when not all conditions are satisfied",
                self.base.get_ident().get_rel_name(),
            );
        }
        self.result = Some(self.schematic.build(&mut self.base, arena));
    }

    fn check_prefinalize(&self) -> Result<(), String> {
        if self.base.get_basic_nodes_i().is_empty() && self.base.get_sub_blocks_i().is_empty() {
            return Err("zync block must hold at least one basic node or basic-flow sub-block".to_string());
        }
        if !self.base.get_con_blocks_i().is_empty() {
            return Err("zync block does not support con blocks".to_string());
        }
        Ok(())
    }

    fn summarize_as_block(&self) -> NodeWrap {
        self.result.clone().expect("zync block has not been built")
    }
}

impl Identifiable for FlowBlockZync {
    fn get_ident_base    (&self)     -> &IdentBase     { self.base.get_ident_ref().get_ident_base() }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.base.get_ident_mut().get_ident_base_mut() }
}
