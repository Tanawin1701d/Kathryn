use crate::model::common::identifier::Identifiable;
use crate::model::complex_hardware::common::ccp_ident::{CcpIdent, CcpType};
use crate::model::flow_block::flow_block_base::{ExtSigType, FlowBlockBase};
use crate::model::flow_block::node_wrap::NodeWrap;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_base::IN_CONSIST_CYCLE_USED;
use crate::model::nodes::ncp_ident::NcpIdent;

// Node / flow-block topology built by `build` (edge label = dependency condition;
// unlabelled edge = unconditional dependency):
//
//                          (upper entry, added later by parent)
//                                       |
//         start node ───────────────────┐
//                                       v
//   ┌────────────────────────────►  [ pseudo ]  (OR join, the entrance)
//   │                              /     ▲      \
//   │              ~master_req    /      │       \   master_req
//   │                           v        │        v
//   │                   [ wait4syn ]     │   ┌───────────────────────┐
//   │                   (state node)     │   │  sub-block (NodeWrap) │
//   │                        │           │   │   entrances ......... │
//   │                        └───────────┤   │       │               │
//   │                                    │   │       v               │
//   │                                    └───┤    exit node          │
//   │                                        └───────────────────────┘
//   │
//   └── master-ack(arb) = pseudo
//
//   result NodeWrap: entrance = pseudo, exit = [ dummy ] (OprNode bound to 0).

// A pipeline schematic owns the CCP that backs the pipeline's control gadget
// (its req/ack/sync wiring) and drives the block's node graph.  `prebuild`
// creates the CCP-side hardware before any nodes exist; `build` wires the node
// graph and returns the block's NodeWrap.
#[derive(Clone, Debug)]
pub struct PipSchematic {
    arb_i: CcpIdent,
}

impl PipSchematic {
    pub fn new(ccp_i: CcpIdent) -> Self {
        assert_eq!(ccp_i.get_ccp_type(), CcpType::Arb, "PipSchematic::new: ccp must be an Arb");
        Self { arb_i: ccp_i }
    }

    pub fn get_arb_i(&self) -> CcpIdent { self.arb_i }

    // for the reset and hold node, the flow block will join with the signal
    //
    // Wire the pipeline node graph and produce its NodeWrap.
    // Node triggers and the arb signal assignment are wired by the caller later.
    pub fn build(&mut self, base: &mut FlowBlockBase, arena: &mut ModelArena) -> NodeWrap {
        // 1. a pipeline holds exactly one sub-block — no basic nodes, no con blocks.
        assert_eq!(base.get_sub_blocks_i().len(), 1, "pipeline flow block must have exactly one sub-block");
        assert!(base.get_basic_nodes_i().is_empty(), "pipeline flow block does not support basic nodes");
        assert!(base.get_con_blocks_i().is_empty(),  "pipeline flow block does not support con blocks");

        let id = base.get_ident().get_global_id();

        // The arb master-request wire (and its inverse) gate the two branches.
        let master_req_i = {
            let arb = arena.take_arb(self.arb_i);
            let r   = arb.get_master_req_wire_i();
            arena.replace_back_arb(arb);
            r
        };
        let no_master_req_i = arena.make_expression_single(
            false, &format!("pip_no_mreq_{}", id), LogicOp::BitwiseInvr, master_req_i, None,
        );

        // 2. state node (wait-for-sync) + pseudo node (the entrancer / OR re-arm).
        // The wait4syn sync node must NOT inherit the block hold: the arb already
        // gates every grant on hold via the master-ack, so holding the sync node
        // too would double-freeze the pipeline.  Copy the trigger minus its hold.
        let wait4syn_i = arena.make_state_node(&format!("pip_wait4syn_{}", id));
        let mut wait_trigger = base.get_ext_trigger_node().clone();
        wait_trigger.hold_node_i = None;
        arena.init_node_trigger(wait4syn_i, &wait_trigger, false);
        base.add_sys_node(wait4syn_i);

        let pseudo_i = arena.make_pseudo_node(&format!("pip_entrance_{}", id), 1, LogicOp::BitwiseOr);
        arena.init_node_trigger(pseudo_i, base.get_ext_trigger_node(), false);
        base.add_sys_node(pseudo_i);

        // 3. resolve the single sub-block into its NodeWrap.
        let sub_block_i = base.get_sub_blocks_i()[0];
        let sub_wrap    = arena.summarize_flow_block(sub_block_i);

        // 4. wait4syn fires when entered while the arb is idle (no master request).
        arena.add_depend_node_to_ncp(wait4syn_i, pseudo_i, Some(no_master_req_i));

        // 5. the sub-block runs when entered while the arb has a master request.
        sub_wrap.add_dep_to_entrances(arena, pseudo_i, Some(master_req_i));

        // 6. pseudo (OR) re-arms from the sub-block exit, the wait4syn node, or
        //    the block's internal start node; the upper-level entry dep is added
        //    later when the parent assigns this entrance.
        arena.add_depend_node_to_ncp(pseudo_i, sub_wrap.get_exit_node_i(), None);
        arena.add_depend_node_to_ncp(pseudo_i, wait4syn_i, None);
        if let Some(start_i) = base.get_int_node(ExtSigType::Start) {
            arena.add_depend_node_to_ncp(pseudo_i, start_i, None);
        }


        // 7. result: pseudo node is the entrance; a zero-bound dummy opr node is the exit.
        let zero_i  = arena.make_val(false, &format!("pip_zero_{}", id), 1, 0);
        let dummy_i = arena.make_opr_node(&format!("pip_dummy_exit_{}", id), zero_i);
        base.add_sys_node(dummy_i);

        // 8. bind the arb master-ack from the entrance pseudo node.
        self.set_arb_master_ack(arena, pseudo_i);

        // 9. assign nodes.  pseudo is the result entrance → prelim only (the parent
        //    calls assign_final via assign_entrance_nodes); wait4syn and the sub-block
        //    entrances are internal so they get the full assign.  The dummy exit opr
        //    node is never assigned (it carries no update event).
        arena.assign_ncp_node(pseudo_i, true, false);
        arena.assign_ncp_node(wait4syn_i, true, true);
        sub_wrap.assign_entrance_nodes(arena);

        // 10. produce a result for next stage
        let mut result = NodeWrap::new();
        result.add_entrance_node_i(pseudo_i);
        result.set_exit_node_i    (dummy_i);
        result.set_cycle_used     (IN_CONSIST_CYCLE_USED);
        result
    }

    // arb master-ack = pseudo (the entrance node) only.  Reset / hold are not
    // gated here — they are handled on the nodes themselves.
    fn set_arb_master_ack(&self, arena: &mut ModelArena, pseudo_i: NcpIdent) {
        let ack_src_i = arena.get_node_exit_opr(&pseudo_i);
        arena.arb_set_master_ack_src(self.arb_i, ack_src_i);
    }
}
