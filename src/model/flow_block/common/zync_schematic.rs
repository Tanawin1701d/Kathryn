use crate::model::common::identifier::Identifiable;
use crate::model::complex_hardware::common::ccp_ident::{CcpIdent, CcpType};
use crate::model::flow_block::flow_block_base::{ExtSigType, FlowBlock, FlowBlockBase};
use crate::model::flow_block::node_wrap::NodeWrap;
use crate::model::hw_component::common::hcp_assign::HcpAssignable;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::hw_component::common::slice::Slice;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_base::IN_CONSIST_CYCLE_USED;

// Node / arb topology built by `build` (edge label = dependency condition;
// unlabelled edge = unconditional dependency):
//
//                    (upper entry, added later by parent)
//                                  |
//        int-start ────────────────┐
//                                   v
//          ┌──────── ~ack ──────► [ state node ]  (request holder, the entrance)
//          │                       /     │      \
//          └──────────────────────┘      │       \  exit-opr drives arb REQ[ch]
//                                         │        └────────────────────────────►
//                                  ack    │
//                                         v
//                                   [ basic asm node ]  (fires on grant: state & ack)
//
//   result NodeWrap: entrance = state node, exit = state node (the node that
//   triggers the basic node, per the spec).
//
// A zync ("synchronise") block parks a request on one arbiter channel: it raises
// REQ from its state node and re-arms (holds REQ) every cycle the state is set
// but the channel has not yet granted ACK.  When the arbiter grants ACK every
// work node (any number of basic AsmNodes and/or basic-flow sub-blocks each
// summarised to one AsmNode) is assigned in that cycle, all gated by the same
// state node.
#[derive(Clone, Debug)]
pub struct ZyncSchematic {
    arb_i      : CcpIdent,   // the arbiter CCP this block contends on
    channel_i  : usize,      // the arb leaf (channel) index this block owns
}

impl ZyncSchematic {
    pub fn new(arb_i: CcpIdent, channel_i: usize) -> Self {
        assert_eq!(arb_i.get_ccp_type(), CcpType::Arb,
                   "ZyncSchematic::new: ccp must be an Arb");
        Self { arb_i, channel_i }
    }

    pub fn get_arb_i    (&self) -> CcpIdent { self.arb_i     }
    pub fn get_channel_i(&self) -> usize    { self.channel_i }

    // Wire the zync node graph and produce its NodeWrap.  The upper-level entry
    // dep onto the state node, and the state node's final assign, are added by
    // the parent later (the state node is the result entrance).
    pub fn build(&mut self, base: &mut FlowBlockBase, arena: &mut ModelArena) -> NodeWrap {
        // ---- stage 0: constraints — at least one work item, no con/sub-flow ----
        assert!(base.get_con_blocks_i().is_empty(), "zync flow block does not support con blocks");
        let (basic_flow_i, sub_flow_i) = base.scan_sub_blocks_by_policy();
        assert!(sub_flow_i.is_empty(), "zync flow block supports only basic nodes or basic-flow sub-blocks");
        assert!(!base.get_basic_nodes_i().is_empty() || !basic_flow_i.is_empty(),
                "zync flow block must hold at least one basic node or basic-flow sub-block");

        let id = base.get_ident().get_global_id();

        // ---- stage 1: build the single unified work AsmNode -----------------
        // Merge every basic AsmNode and every basic-flow sub-block into one
        // ordered, per-target-unified AssignMeta list (this preserves the
        // original insertion order between direct nodes and basic-flow blocks),
        // then build a single clean AsmNode from it and wire its trigger / clk.
        let grp_asms   = base.gen_unified_asm_meta_flat(arena);
        let asm_node_i = arena.make_asm_node_many(&format!("zync_asm_{}", id), &grp_asms);
        arena.init_node_trigger(asm_node_i, base.get_ext_trigger_node(), false);
        //arena.init_asm_node_clk_src(asm_node_i); you don't have to create clk source because the flow block give you already

        // ---- stage 2: the request-holding state node ------------------------
        let state_i = arena.make_state_node(&format!("zync_state_{}", id));
        arena.init_node_trigger(state_i, base.get_ext_trigger_node(), false);
        base.add_sys_node(state_i);

        // Prelim-assign now so the state node's exit-opr / state-operating signals
        // exist for the arb req wiring and the basic-node gate below.  Final assign
        // is left to the parent (this node is the result entrance).
        arena.assign_ncp_node(state_i, true, false);
        let state_exit_i = arena.get_node_exit_opr(&state_i);

        // ---- stage 3: arb channel handshake wiring --------------------------
        let (req_wire_i, ack_wire_i) = {
            let arb = arena.take_arb(self.arb_i);
            assert!(self.channel_i < arb.leaf_count(),
                    "ZyncSchematic::build: channel index out of range for arb");
            let req_wire_i = arb.get_leaf_req_wire_i(self.channel_i);
            let ack_wire_i = arb.get_leaf_ack_wire_i(self.channel_i);
            arena.replace_back_arb(arb);
            (req_wire_i, ack_wire_i)
        };

        // (2) arb REQ[ch] = state node's exit signal.
        {
            let mut req_wire = arena.take_wire(req_wire_i);
            req_wire.bind_src(state_exit_i, None, Slice::new(0, 1), None, None, None, arena);
            arena.replace_back_wire(req_wire);
        }

        // (3) re-arm: the state is set again while its exit is set and ACK is low.
        let no_ack_i = arena.make_expression_single(
            false, &format!("zync_no_ack_{}", id), LogicOp::BitwiseInvr, ack_wire_i, None,
        );
        arena.add_depend_node_to_ncp(state_i, state_i, Some(no_ack_i));

        // Initial entry from the block's int-start, when present.
        if let Some(start_i) = base.get_int_node(ExtSigType::Start) {
            arena.add_depend_node_to_ncp(state_i, start_i, None);
        }

        // ---- stage 4: gate the unified work node on the grant (state & ACK) --
        // The asm node takes the state node as its single parent depend, gated by
        // ACK; assign_from_state_node folds in (ack & ~hold & ~reset & state_op).
        arena.add_depend_node_to_ncp(asm_node_i, state_i, Some(ack_wire_i));
        arena.assign_asm_from_state_node(asm_node_i);

        // ---- stage 5: summarise — entrance and exit are the state node ------
        // (5) the exit node is the one that triggers the basic node: the state node.
        NodeWrap::with_single_entrance(state_i, state_i, IN_CONSIST_CYCLE_USED)
    }
}
