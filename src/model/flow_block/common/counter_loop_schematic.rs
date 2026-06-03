use crate::model::common::identifier::Identifiable;
use crate::model::flow_block::flow_block_base::{ExtSigType, FlowBlockBase};
use crate::model::flow_block::node_wrap::NodeWrap;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_base::IN_CONSIST_CYCLE_USED;

#[derive(Clone, Debug)]
pub struct CounterLoopSchematic {
    last_loop_cnt : i32,
    loop_id_expr_i: Option<HcpIdent>,
}

impl CounterLoopSchematic {
    pub fn new(last_loop_cnt: i32) -> Self {
        assert!(last_loop_cnt > 0, "counter loop count must be positive");
        Self {
            last_loop_cnt,
            loop_id_expr_i: None,
        }
    }

    pub fn get_loop_id_expr_i(&self) -> HcpIdent {
        self.loop_id_expr_i.expect("counter loop has not been built yet")
    }

    pub fn build(&mut self, base: &mut FlowBlockBase, arena: &mut ModelArena) -> NodeWrap {
        assert_eq!(base.get_sub_blocks_i().len(), 1, "counter loop must have exactly one body block");
        assert!(base.get_con_blocks_i().is_empty(), "counter loop does not support con blocks");

        let id = base.get_ident().get_global_id();

        // 1. Summarize body
        let body_block_i = base.get_sub_blocks_i()[0];
        let body_wrap = arena.summarize_flow_block(body_block_i);
        let body_exit_i = body_wrap.get_exit_node_i();

        // 2. Create counter_node incremented by body_exit only (not ent)
        let counter_node_i = arena.make_counter_node(
            &format!("cloop_cnt_{}", id),
            self.last_loop_cnt,
        );
        arena.init_node_trigger(counter_node_i, base.get_ext_trigger_node(), false);
        base.add_sys_node(counter_node_i);

        // Retrieve the underlying CntReg ident for callers (loop variable)
        let loop_id = {
            let cnt = arena.take_counter_node(counter_node_i);
            let r = cnt.get_cnt_reg_i();
            arena.replace_back_counter_node(cnt);
            r
        };
        self.loop_id_expr_i = Some(loop_id);

        // Wire increment trigger and assign to materialise cnt_at_last
        arena.add_depend_node_to_ncp(counter_node_i, body_exit_i, None);
        arena.assign_ncp_node(counter_node_i);
        let cnt_at_last_i = arena.get_node_exit_opr(&counter_node_i);

        // 3. Create loop_node — fires to start each iteration
        let loop_node_i = arena.make_pseudo_node(&format!("cloop_loop_{}", id), 1, LogicOp::BitwiseOr);
        arena.init_node_trigger(loop_node_i, base.get_ext_trigger_node(), false);
        base.add_sys_node(loop_node_i);

        // Initial entry from int_start
        if let Some(start_i) = base.get_int_node(ExtSigType::Start) {
            arena.add_depend_node_to_ncp(loop_node_i, start_i, None);
        }

        // Loop-back: (body_exit, ~cnt_at_last) → loop_node
        let not_at_last_i = arena.make_expression_single(
            false, &format!("cloop_not_last_{}", id),
            LogicOp::BitwiseInvr,
            cnt_at_last_i,
            None,
        );
        arena.add_depend_node_to_ncp(loop_node_i, body_exit_i, Some(not_at_last_i));

        // Body enters when loop_node fires
        body_wrap.add_dep_to_entrances(arena, loop_node_i, None);

        // 4. Create exit_node — fires when body_exit AND cnt_at_last
        let exit_i = arena.make_pseudo_node(&format!("cloop_exit_{}", id), 1, LogicOp::BitwiseOr);
        arena.init_node_trigger(exit_i, base.get_ext_trigger_node(), false);
        base.add_sys_node(exit_i);
        arena.add_depend_node_to_ncp(exit_i, body_exit_i, Some(cnt_at_last_i));

        // 5. Assign remaining internal nodes — do NOT assign loop_node_i; it is the result entrance,
        //    the parent is responsible for assigning it
        arena.assign_ncp_node(exit_i);
        body_wrap.assign_entrance_nodes(arena);

        // 6. Cycle count: body_cycle * iterations (or inconsistent)
        let body_cycle = body_wrap.get_cycle_used();
        let cycle_used = if body_cycle == IN_CONSIST_CYCLE_USED {
            //// no need to consider the amount node in counter node
            IN_CONSIST_CYCLE_USED
        } else {
            self.last_loop_cnt * body_cycle
        };

        // 7. Build result
        NodeWrap::with_single_entrance(loop_node_i, exit_i, cycle_used)
    }
}
