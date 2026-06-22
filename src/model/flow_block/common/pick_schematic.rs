use crate::model::common::identifier::Identifiable;
use crate::model::flow_block::flow_block_base::{ExtSigType, FlowBlockBase};
use crate::model::flow_block::node_wrap::{NodeWrap, NodeWrapCycleDet};
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_base::{add_logic_with_output, IN_CONSIST_CYCLE_USED};

/// Wires a `pick` container: a single combinational entry node fans into every
/// branch (each gated on its own RAW condition — no mutual-exclusion chaining),
/// and a plain OR exit node collects every branch exit. The optional default
/// branch (pidef) is gated on `~cond1 & ~cond2 & …` (all pifs false); when there
/// is no default the entry falls straight through to the exit on that same
/// "nothing matched" signal. The exit is NOT auto-synchronized.
#[derive(Clone, Debug, Default)]
pub struct PickSchematic;

impl PickSchematic {
    pub fn new() -> Self { Self }

    pub fn build(&mut self, base: &mut FlowBlockBase, arena: &mut ModelArena) -> NodeWrap {
        assert!(!base.get_sub_blocks_i().is_empty(), "pick block must have at least one pif branch");

        let id = base.get_ident().get_global_id();
        let mut cycle_det = NodeWrapCycleDet::new();

        // 1. entry node — combinational pseudo-OR that triggers the picked branch
        let entry_node_i = arena.make_pseudo_node(&format!("pick_entry_{}", id), 1, LogicOp::BitwiseOr);
        arena.init_node_trigger(entry_node_i, base.get_ext_trigger_node(), false);
        if let Some(start_i) = base.get_int_node(ExtSigType::Start) {
            arena.add_depend_node_to_ncp(entry_node_i, start_i, None);
        }
        base.add_sys_node(entry_node_i);
        arena.assign_ncp_node(entry_node_i, true, false);

        // 2. exit node — plain OR over the branch exits (no synchronizer)
        let exit_i = arena.make_pseudo_node(&format!("pick_exit_{}", id), 1, LogicOp::BitwiseOr);
        arena.init_node_trigger(exit_i, base.get_ext_trigger_node(), false);
        base.add_sys_node(exit_i);

        // 3. gate each branch behind its raw condition; build the "all false" signal
        let branch_ids: Vec<_> = base.get_sub_blocks_i().to_vec();
        let mut all_false  : Option<HcpIdent> = None;
        let mut default_wrap: Option<NodeWrap> = None;
        let mut pif_wraps  : Vec<NodeWrap> = Vec::new();

        for branch_i in branch_ids {
            let branch    = arena.take_flow_block_pick_if(branch_i);
            let condition = branch.get_pick_condition();
            arena.replace_back_flow_block_pick_if(branch);

            let wrap = arena.summarize_flow_block(branch_i);
            cycle_det.add_cycle(wrap.get_cycle_used());

            match condition {
                Some(cond_i) => {
                    wrap.add_dep_to_entrances(arena, entry_node_i, Some(cond_i));
                    let inv_i = arena.make_expression_single(
                        false, &format!("pick_not_{}_{}", id, pif_wraps.len()),
                        LogicOp::BitwiseInvr, cond_i, None,
                    );
                    all_false = add_logic_with_output(arena, all_false, Some(inv_i), LogicOp::BitwiseAnd);
                    pif_wraps.push(wrap);
                }
                None => {
                    assert!(default_wrap.is_none(), "pick block accepts at most one pidef default branch");
                    default_wrap = Some(wrap);
                }
            }
        }

        // 4. OR every branch exit into the shared exit; gate the default / fall-through
        for wrap in &pif_wraps {
            arena.add_depend_node_to_ncp(exit_i, wrap.get_exit_node_i(), None);
        }
        if let Some(def_wrap) = &default_wrap {
            def_wrap.add_dep_to_entrances(arena, entry_node_i, all_false);
            arena.add_depend_node_to_ncp(exit_i, def_wrap.get_exit_node_i(), None);
        } else if let Some(fall_cond) = all_false {
            // no default: pick exits directly when nothing matched
            arena.add_depend_node_to_ncp(exit_i, entry_node_i, Some(fall_cond));
        }

        if base.get_int_node(ExtSigType::Start).is_some() {
            cycle_det.add_cycle(IN_CONSIST_CYCLE_USED);
        }

        // 5. assign internal nodes — entry stays prelim-only (parent assigns it)
        arena.assign_ncp_node(exit_i, true, true);
        for wrap in &pif_wraps {
            wrap.assign_entrance_nodes(arena);
        }
        if let Some(def_wrap) = &default_wrap {
            def_wrap.assign_entrance_nodes(arena);
        }

        NodeWrap::with_single_entrance(entry_node_i, exit_i, cycle_det.get_same_cycle_horizon())
    }
}
