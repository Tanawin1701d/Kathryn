use crate::model::common::identifier::Identifiable;
use crate::model::complex_hardware::common::ccp_ident::{CcpIdent, CcpType};
use crate::model::flow_block::flow_block_base::{ExtSigType, FlowBlock, FlowBlockBase};
use crate::model::flow_block::node_wrap::NodeWrap;
use crate::model::hw_component::common::hcp_assign::HcpAssignable;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::hw_component::common::slice::Slice;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_base::{add_logic_with_output, IN_CONSIST_CYCLE_USED};

// One arb contended on by a zync block: a channel (leaf) on `arb_i`, optionally
// gated by a raw `condition_i` signal.  The channel's REQ is `state_exit & cond`
// and its grant contribution is `ack & cond` (cond defaults to 1 when absent).
#[derive(Clone, Copy, Debug)]
pub struct ZyncArbBind {
    arb_i      : CcpIdent,         // the arbiter CCP this bind contends on
    channel_i  : usize,           // the arb leaf (channel) index this bind owns
    condition_i: Option<HcpIdent>, // optional REQ/grant gate (None = always)
}

impl ZyncArbBind {
    pub fn new(arb_i: CcpIdent, channel_i: usize, condition_i: Option<HcpIdent>) -> Self {
        assert_eq!(arb_i.get_ccp_type(), CcpType::Arb, "ZyncArbBind::new: ccp must be an Arb");
        Self { arb_i, channel_i, condition_i }
    }

    pub fn get_arb_i    (&self) -> CcpIdent          { self.arb_i       }
    pub fn get_channel_i(&self) -> usize             { self.channel_i   }
    pub fn get_cond_i   (&self) -> Option<HcpIdent>  { self.condition_i }
}

// How a multi-arb zync decides it has been granted (the state-exit / work-node
// gate): `Any` fires when SOME bind's `ack & cond` is high (OR), `All` fires only
// when EVERY bind's `ack & cond` is high (AND).  A single-arb zync is `Any` (the
// one-term OR and AND are identical).
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum ZyncSyncMode {
    Any, // "for some" — OR over the binds' (ack & cond)
    All, // "for all"  — AND over the binds' (ack & cond)
}

impl ZyncSyncMode {
    // The fold operator used to aggregate the per-bind grant terms.
    fn agg_op(self) -> LogicOp {
        match self {
            ZyncSyncMode::Any => LogicOp::BitwiseOr,
            ZyncSyncMode::All => LogicOp::BitwiseAnd,
        }
    }
}

// Node / arb topology built by `build` (edge label = dependency condition;
// unlabelled edge = unconditional dependency):
//
//                    (upper entry, added later by parent)
//                                  |
//        int-start ────────────────┐
//                                   v
//          ┌─────── ~grant ─────► [ state node ]  (request holder, the entrance)
//          │                       /     │      \
//          └──────────────────────┘      │       \  exit-opr & cond_k drives REQ[ch_k]
//                                         │        └────────────────────────────►
//                                 grant   │   grant = Any/All over (ack_k & cond_k)
//                                         v
//                                   [ basic asm node ]  (fires on grant: state & grant)
//
//   result NodeWrap: entrance = state node, exit = state node (the node that
//   triggers the basic node, per the spec).
//
// A zync ("synchronise") block parks a request on one OR MORE arbiter channels:
// it raises REQ from its state node (gated per-bind by an optional condition) and
// re-arms (holds the state) every cycle it is set but has not yet been granted.
// The grant is `Any`/`All` over the binds' `ack & cond`; when it fires every work
// node (any number of basic AsmNodes and/or basic-flow sub-blocks each summarised
// to one AsmNode) is assigned in that cycle, all gated by the same state node.
#[derive(Clone, Debug)]
pub struct ZyncSchematic {
    binds: Vec<ZyncArbBind>, // the arb channels this block contends on (>= 1)
    mode : ZyncSyncMode,     // how the binds' grants combine (Any / All)
}

impl ZyncSchematic {
    // The one constructor: contend on every bind, combining grants per `mode`.  A
    // single-arb zync is just a one-element `binds` (its `mode` is irrelevant — Any
    // and All coincide for one term).
    pub fn new(binds: Vec<ZyncArbBind>, mode: ZyncSyncMode) -> Self {
        assert!(!binds.is_empty(), "ZyncSchematic::new: at least one arb bind required");
        Self { binds, mode }
    }

    pub fn get_binds(&self) -> &Vec<ZyncArbBind> { &self.binds }
    pub fn get_mode (&self) -> ZyncSyncMode      { self.mode   }

    // Build the zync node graph and return its NodeWrap.  Lays down the unified
    // work AsmNode and the request-holding state node, drives each bind's REQ and
    // folds their grants, then re-arms the state on `~grant` and gates the work
    // node on the aggregate grant.  Two pieces are deliberately left to the parent:
    // the upper-level entry dependency onto the state node, and the state node's
    // final assign — which is why the state node is the returned entrance (and exit).
    pub fn build(&mut self, base: &mut FlowBlockBase, arena: &mut ModelArena) -> NodeWrap {
        // ---- stage 0: constraints — at least one work item, no con/sub-flow ----
        assert!(base.get_con_blocks_i().is_empty(), "zync flow block does not support con blocks");
        let (basic_flow_i, sub_flow_i) = base.scan_sub_blocks_by_policy();
        assert!(sub_flow_i.is_empty(), "zync flow block supports only basic nodes or basic-flow sub-blocks");
        assert!(!base.get_basic_nodes_i().is_empty() || !basic_flow_i.is_empty(),
                "zync flow block must hold at least one basic node or basic-flow sub-block");

        let block_id = base.get_ident().get_global_id();

        // ---- stage 1: build the single unified work AsmNode -----------------
        // Merge every basic AsmNode and every basic-flow sub-block into one
        // ordered, per-target-unified AssignMeta list (this preserves the
        // original insertion order between direct nodes and basic-flow blocks),
        // then build a single clean AsmNode from it and wire its trigger / clk.
        let grp_asms   = base.gen_unified_asm_meta_flat(arena);
        let asm_node_i = arena.make_asm_node_many(&format!("zync_asm_{}", block_id), &grp_asms);
        arena.init_node_trigger(asm_node_i, base.get_ext_trigger_node(), false);
        //arena.init_asm_node_clk_src(asm_node_i); you don't have to create clk source because the flow block give you already

        // ---- stage 2: the request-holding state node ------------------------
        let state_i = arena.make_state_node(&format!("zync_state_{}", block_id));
        arena.init_node_trigger(state_i, base.get_ext_trigger_node(), false);
        base.add_sys_node(state_i);

        // Prelim-assign now so the state node's exit-opr / state-operating signals
        // exist for the arb req wiring and the basic-node gate below.  Final assign
        // is left to the parent (this node is the result entrance).
        arena.assign_ncp_node(state_i, true, false);
        let state_exit_i = arena.get_node_exit_opr(&state_i);

        // ---- stage 3: per-bind REQ wiring + grant-term accumulation ---------
        // For each arb bind: drive REQ[ch_k] = state_exit & cond_k, and fold the
        // grant term (ack_k & cond_k) into the aggregate per the sync mode (Any =>
        // OR, All => AND).  A bind with no condition uses state_exit / ack directly.
        let agg_op = self.mode.agg_op();
        let mut grant_i: Option<HcpIdent> = None;

        for (bind_idx, bind) in self.binds.iter().enumerate() {
            let term_i = Self::do_req_and_get_grant(arena, bind, bind_idx, block_id, state_exit_i);
            grant_i = add_logic_with_output(arena, grant_i, Some(term_i), agg_op);
        }

        let grant_i = grant_i.expect("zync block must contend on at least one arb bind");

        // (3) re-arm: hold the state while it is set but not yet granted.
        let no_grant_i = arena.make_expression_single(
            false, &format!("zync_no_grant_{}", block_id), LogicOp::BitwiseInvr, grant_i, None,
        );
        arena.add_depend_node_to_ncp(state_i, state_i, Some(no_grant_i));

        // Initial entry from the block's int-start, when present.
        if let Some(start_i) = base.get_int_node(ExtSigType::Start) {
            arena.add_depend_node_to_ncp(state_i, start_i, None);
        }

        // ---- stage 4: gate the unified work node on the grant (state & grant) -
        // The asm node takes the state node as its single parent depend, gated by
        // the aggregate grant; assign_from_state_node folds in
        // (grant & ~hold & ~reset & state_op).
        arena.add_depend_node_to_ncp(asm_node_i, state_i, Some(grant_i));
        arena.assign_asm_from_state_node(asm_node_i);

        // ---- stage 5: summarise — entrance and exit are the state node ------
        // (5) the exit node is the one that triggers the basic node: the state node.
        NodeWrap::with_single_entrance(state_i, state_i, IN_CONSIST_CYCLE_USED)
    }

    // Wire one arb bind: drive its REQ = `state_exit & cond` and return its grant
    // term `ack & cond` (cond defaults to 1 when absent, collapsing both to the bare
    // state_exit / ack).  `bind_idx` only disambiguates generated signal names.
    fn do_req_and_get_grant(
        arena       : &mut ModelArena,
        bind        : &ZyncArbBind,
        bind_idx    : usize,
        block_id    : u64,
        state_exit_i: HcpIdent,
    ) -> HcpIdent {
        let (req_wire_i, ack_wire_i) = {
            let arb = arena.take_arb(bind.get_arb_i());
            assert!(bind.get_channel_i() < arb.leaf_count(),
                    "ZyncSchematic::build: channel index out of range for arb");
            let req_wire_i = arb.get_leaf_req_wire_i(bind.get_channel_i());
            let ack_wire_i = arb.get_leaf_ack_wire_i(bind.get_channel_i());
            arena.replace_back_arb(arb);
            (req_wire_i, ack_wire_i)
        };

        // REQ[ch] = state_exit & cond (just state_exit when no cond).
        let req_src_i = match bind.get_cond_i() {
            Some(cond_i) => arena.make_expression(
                false, &format!("zync_req_{}_{}", block_id, bind_idx),
                LogicOp::BitwiseAnd, state_exit_i, cond_i, None, None,
            ),
            None => state_exit_i,
        };
        {
            let mut req_wire = arena.take_wire(req_wire_i);
            req_wire.bind_src(req_src_i, None, Slice::new(0, 1), None, None, None, arena);
            arena.replace_back_wire(req_wire);
        }

        // grant term = ack & cond (just ack when no cond).
        match bind.get_cond_i() {
            Some(cond_i) => arena.make_expression(
                false, &format!("zync_grant_term_{}_{}", block_id, bind_idx),
                LogicOp::BitwiseAnd, ack_wire_i, cond_i, None, None,
            ),
            None => ack_wire_i,
        }
    }
}
