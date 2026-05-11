use crate::model::common::identifier::Identifiable;
use crate::model::flow_block::flow_block_base::{ExtSigType, FlowBlockBase};
use crate::model::flow_block::node_wrap::NodeWrap;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_base::IN_CONSIST_CYCLE_USED;

#[derive(Clone, Debug)]
pub struct DoWhileSchematic {
    cond_i: HcpIdent,
}

impl DoWhileSchematic {
    pub fn new(cond_i: HcpIdent) -> Self {
        Self { cond_i }
    }

    pub fn build(&mut self, base: &mut FlowBlockBase, arena: &mut ModelArena) -> NodeWrap {
        assert!(!base.get_sub_blocks_i().is_empty(), "do-while block must have one body block");
        assert!(base.get_con_blocks_i().is_empty(), "do-while block does not support con blocks");

        let id = base.get_ident().get_global_id();

        // 1. Summarize body — its entrance IS the do-while entrance
        let body_block_i = base.get_sub_blocks_i()[0];
        let body_wrap = arena.summarize_flow_block(body_block_i);
        let body_exit_i = body_wrap.get_exit_node_i();

        // 2. Wire body entrance triggers
        // Optional initial trigger from int_start (for interrupt-driven re-entry)
        if let Some(start_i) = base.get_int_node(ExtSigType::Start) {
            for &entrance_i in body_wrap.get_entrance_nodes_i() {
                arena.add_depend_node_to_ncp(entrance_i, start_i, None);
            }
        }
        // Loop-back: body_exit (with cond_i) feeds back to body entrance
        for &entrance_i in body_wrap.get_entrance_nodes_i() {
            arena.add_depend_node_to_ncp(entrance_i, body_exit_i, Some(self.cond_i));
        }

        // 3. Create exit_node — fires when body exits with condition false
        let not_cond_i = arena.make_expression(
            &format!("dowhile_not_cond_{}", id),
            LogicOp::BitwiseInvr,
            self.cond_i,
            HcpIdent::default(),
            None, None,
        );
        let exit_i = arena.make_pseudo_node(&format!("dowhile_exit_{}", id), 1, LogicOp::BitwiseOr);
        arena.init_node_trigger(exit_i, base.get_ext_trigger_node(), false);
        base.add_sys_node(exit_i);
        arena.add_depend_node_to_ncp(exit_i, body_exit_i, Some(not_cond_i));

        // 4. Assign exit_node; body entrance nodes are returned to parent for assignment
        arena.assign_ncp_node(exit_i);

        // 5. Build result — body's entrances become the do-while entrances
        let mut result = NodeWrap::new();
        result.add_entrance_nodes_i(body_wrap.get_entrance_nodes_i());
        result.set_exit_node_i(exit_i);
        result.set_cycle_used(IN_CONSIST_CYCLE_USED);
        result
    }
}
