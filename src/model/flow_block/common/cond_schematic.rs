use crate::model::common::identifier::Identifiable;
use crate::model::flow_block::flow_block_base::{ExtSigType, FlowBlockBase};
use crate::model::flow_block::node_wrap::{NodeWrap, NodeWrapCycleDet};
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_base::{add_logic_with_output, IN_CONSIST_CYCLE_USED};

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum CondMode {
    Combinatorial, // CIF — pseudo cond node (same-cycle)
    Sequential,    // SIF — state cond node (registered)
}

#[derive(Clone, Debug)]
pub struct CondSchematic {
    mode  : CondMode,
    cond_i: HcpIdent,
}

impl CondSchematic {
    pub fn new(mode: CondMode, cond_i: HcpIdent) -> Self {
        Self { mode, cond_i }
    }

    pub fn build(&mut self, base: &mut FlowBlockBase, arena: &mut ModelArena) -> NodeWrap {
        assert!(!base.get_sub_blocks_i().is_empty(), "cond block must have at least one body block");

        // init id and cycle deteminer
        let id = base.get_ident().get_global_id();
        let mut cycle_det = NodeWrapCycleDet::new();

        // 1. Create cond_node — entry point that evaluates the condition
        let cond_node_i = match self.mode {
            CondMode::Combinatorial => arena.make_pseudo_node(&format!("cif_cond_{}", id), 1, LogicOp::BitwiseOr),
            CondMode::Sequential    => arena.make_state_node(&format!("sif_cond_{}", id)),
        };
        arena.init_node_trigger(cond_node_i, base.get_ext_trigger_node(), false);
        if let Some(start_i) = base.get_int_node(ExtSigType::Start) {
            arena.add_depend_node_to_ncp(cond_node_i, start_i, None);
        }
        base.add_sys_node(cond_node_i);

        // 2. Summarize the main body (sub_blocks[0]) and gate it behind cond_i
        let main_block_i = base.get_sub_blocks_i()[0];
        let main_wrap = arena.summarize_flow_block(main_block_i);
        main_wrap.add_dep_to_entrances(arena, cond_node_i, Some(self.cond_i));
        cycle_det.add_cycle(main_wrap.get_cycle_used());

        // 3. Mutual-exclusion chain: prev_false = ~cond_i
        let inv_main = arena.make_expression(
            &format!("cif_not_cond_{}", id),
            LogicOp::BitwiseInvr,
            self.cond_i,
            HcpIdent::default(),
            None, None,
        );
        let mut prev_false: Option<HcpIdent> = Some(inv_main);

        // 4. Process con_blocks (elif / else)

        let con_block_ids: Vec<_> = base.get_con_blocks_i().to_vec();
        let mut con_wraps: Vec<NodeWrap> = Vec::new();

        for (i, &con_i) in con_block_ids.iter().enumerate() {
            let con_condition = arena.get_flow_block(con_i).get_con_condition();
            let con_wrap = arena.summarize_flow_block(con_i);

            let gated_cond: Option<HcpIdent> = match con_condition {
                Some(elif_cond) => {
                    // elif: gate = elif_cond & prev_false; advance prev_false
                    let gated = add_logic_with_output(arena, Some(elif_cond), prev_false, LogicOp::BitwiseAnd);
                    let inv_elif = arena.make_expression(
                        &format!("cif_not_elif_{}_{}", id, i),
                        LogicOp::BitwiseInvr,
                        elif_cond,
                        HcpIdent::default(),
                        None, None,
                    );
                    prev_false = add_logic_with_output(arena, prev_false, Some(inv_elif), LogicOp::BitwiseAnd);
                    gated
                }
                None => {
                    // else: gate = prev_false (consume — else is always last)
                    prev_false.take()
                }
            };

            con_wrap.add_dep_to_entrances(arena, cond_node_i, gated_cond);

            cycle_det.add_cycle(con_wrap.get_cycle_used());
            con_wraps.push(con_wrap);
        }

        // 5. Create exit_node (OR pseudo) collecting all path exits // pick one!
        let exit_i = arena.make_pseudo_node(&format!("cif_exit_{}", id), 1, LogicOp::BitwiseOr);
        arena.init_node_trigger(exit_i, base.get_ext_trigger_node(), false);
        base.add_sys_node(exit_i);

        arena.add_depend_node_to_ncp(exit_i, main_wrap.get_exit_node_i(), None);
        for con_wrap in &con_wraps {
            arena.add_depend_node_to_ncp(exit_i, con_wrap.get_exit_node_i(), None);
        }

        // Fall-through: when no else branch, cond_node exits directly when all conditions false
        if let Some(fall_cond) = prev_false {
            arena.add_depend_node_to_ncp(exit_i, cond_node_i, Some(fall_cond));
        }

        if base.get_int_node(ExtSigType::Start).is_some() {
            cycle_det.add_cycle(IN_CONSIST_CYCLE_USED);
        }

        // 6. Assign internal nodes — do NOT assign cond_node_i; it is the result entrance,
        //    the parent is responsible for assigning it
        arena.assign_ncp_node(exit_i);
        main_wrap.assign_entrance_nodes(arena);
        for con_wrap in &con_wraps {
            con_wrap.assign_entrance_nodes(arena);
        }

        // 7. Build result
        NodeWrap::with_single_entrance(cond_node_i, exit_i, cycle_det.get_same_cycle_horizon())
    }
}
