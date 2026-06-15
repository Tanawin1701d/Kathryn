use crate::model::common::identifier::Identifiable;
use crate::model::flow_block::flow_block_base::{ExtSigType, FlowBlockBase};
use crate::model::flow_block::node_wrap::NodeWrap;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::slice::Slice;
use crate::model::model_arena::ModelArena;

// A wait block is a leaf: it owns exactly one wait node which is both the
// entrance and the exit of its NodeWrap.  `Cond` waits until `cond_i[cond_sl]`
// fires (WaitCondNode); `Cycle` waits a fixed number of clocks (WaitCycleNode).
#[derive(Clone, Debug)]
pub enum WaitMode {
    Cond  { cond_i: HcpIdent, cond_sl: Slice },
    Cycle { cycle : i32 },
}

#[derive(Clone, Debug)]
pub struct WaitSchematic {
    mode: WaitMode,
}

impl WaitSchematic {
    pub fn new_cond (cond_i: HcpIdent, cond_sl: Slice) -> Self { Self { mode: WaitMode::Cond { cond_i, cond_sl } } }
    pub fn new_cycle(cycle : i32)                      -> Self { Self { mode: WaitMode::Cycle { cycle } } }

    pub fn build(&mut self, base: &mut FlowBlockBase, arena: &mut ModelArena) -> NodeWrap {
        assert!(base.get_sub_blocks_i().is_empty(), "wait block has no body sub-block");
        assert!(base.get_con_blocks_i().is_empty(), "wait block does not support con blocks");

        let id = base.get_ident().get_global_id();

        // 1. Create the single wait node (entrance == exit).
        let wait_node_i = match &self.mode {
            WaitMode::Cond  { cond_i, cond_sl } => arena.make_wait_cond_node (&format!("scwait_{}", id), *cond_i, *cond_sl),
            WaitMode::Cycle { cycle }           => arena.make_wait_cycle_node(&format!("sywait_{}", id), *cycle),
        };
        arena.init_node_trigger(wait_node_i, base.get_ext_trigger_node(), false);

        // Initial entry from the block's int_start, when present.
        if let Some(start_i) = base.get_int_node(ExtSigType::Start) {
            arena.add_depend_node_to_ncp(wait_node_i, start_i, None);
        }
        base.add_sys_node(wait_node_i);

        // 2. Prelim only — the wait node is the result entrance, so the parent
        //    runs assign_final via NodeWrap::assign_entrance_nodes.
        arena.assign_ncp_node(wait_node_i, true, false);

        // 3. Build result — entrance and exit are the same wait node.
        let cycle_used = arena.get_node_cycle_used(&wait_node_i);
        NodeWrap::with_single_entrance(wait_node_i, wait_node_i, cycle_used)
    }
}
