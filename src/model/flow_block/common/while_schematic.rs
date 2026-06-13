use crate::model::common::identifier::Identifiable;
use crate::model::flow_block::flow_block_base::{ExtSigType, FlowBlockBase};
use crate::model::flow_block::node_wrap::NodeWrap;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_base::IN_CONSIST_CYCLE_USED;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum LoopMode {
    Combinatorial, // CWHILE — pseudo cond node
    Sequential,    // SWHILE — state cond node
}

#[derive(Clone, Debug)]
pub struct WhileSchematic {
    mode  : LoopMode,
    cond_i: HcpIdent,
}

impl WhileSchematic {
    pub fn new(mode: LoopMode, cond_i: HcpIdent) -> Self {
        Self { mode, cond_i }
    }

    pub fn build(&mut self, base: &mut FlowBlockBase, arena: &mut ModelArena) -> NodeWrap {
        assert!(!base.get_sub_blocks_i().is_empty(), "while block must have one body block");
        assert!(base.get_con_blocks_i().is_empty(), "while block does not support con blocks");

        let id = base.get_ident().get_global_id();

        // 1. Create cond_node — re-evaluated before every iteration
        let cond_node_i = match self.mode {
            LoopMode::Combinatorial => arena.make_pseudo_node(&format!("cwhile_cond_{}", id), 1, LogicOp::BitwiseOr),
            LoopMode::Sequential    => arena.make_state_node(&format!("swhile_cond_{}", id)),
        };
        arena.init_node_trigger(cond_node_i, base.get_ext_trigger_node(), false);
        if let Some(start_i) = base.get_int_node(ExtSigType::Start) {
            arena.add_depend_node_to_ncp(cond_node_i, start_i, None);
        }
        base.add_sys_node(cond_node_i);
        arena.assign_ncp_node(cond_node_i, true, false);

        // 2. Summarize body
        let body_block_i = base.get_sub_blocks_i()[0];
        let body_wrap = arena.summarize_flow_block(body_block_i);
        let body_exit_i = body_wrap.get_exit_node_i();

        // Feedback: body_exit → cond_node (loop-back)
        arena.add_depend_node_to_ncp(cond_node_i, body_exit_i, None);

        // Body enters when condition is true
        body_wrap.add_dep_to_entrances(arena, cond_node_i, Some(self.cond_i));

        // 3. Create exit_node — fires when condition is false
        let not_cond_i = arena.make_expression_single(
            false, &format!("cwhile_not_cond_{}", id),
            LogicOp::BitwiseInvr,
            self.cond_i,
            None,
        );
        let exit_i = arena.make_pseudo_node(&format!("cwhile_exit_{}", id), 1, LogicOp::BitwiseOr);
        arena.init_node_trigger(exit_i, base.get_ext_trigger_node(), false);
        base.add_sys_node(exit_i);
        arena.add_depend_node_to_ncp(exit_i, cond_node_i, Some(not_cond_i));

        // 4. Assign internal nodes — do NOT assign cond_node_i here (final); it is the result
        //    entrance and the parent calls assign_final via assign_entrance_nodes.
        //    Prelim was already called in step 1 so exit_i's assign_final can read cond_node's exit_opr.
        arena.assign_ncp_node(exit_i, true, true);
        body_wrap.assign_entrance_nodes(arena);

        // 5. Build result — cycle_used is always inconsistent due to feedback
        NodeWrap::with_single_entrance(cond_node_i, exit_i, IN_CONSIST_CYCLE_USED)
    }
}
