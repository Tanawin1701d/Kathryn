use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::nodes::ncp_ident::{NcpIdent, NodeType};
use crate::model::model_arena::ModelArena;
use crate::params::MAX_DEPEND_NODES;

pub const NODE_CYCLE_USED_UNKNOWN: i32 = -1;
pub const IN_CONSIST_CYCLE_USED  : i32 = -2;

// ---- NodeTriggerSig ---------------------------------------------------------
//
// Trigger bundle held *by value* in every concrete node, mirroring the HCP-side
// `TriggerSig`/`HasTriggerSig` pattern.  Depend edges store the *node* the
// dependency points at (NcpIdent) plus an optional `condition` operand
// (HcpIdent).  The two distinguished trigger nodes (hold / interrupt-reset)
// are stored as plain `NcpIdent` so consumers can resolve their exit operand
// through `ModelArena::get_node_exit_opr` at `assign` time.

#[derive(Clone, Copy, Debug, PartialEq, Eq, Default)]
pub struct NodeTriggerSig {
    pub hold_node     : Option<NcpIdent>,
    pub int_reset     : Option<NcpIdent>,
    depend_nodes      : [Option<(NcpIdent, Option<HcpIdent>)>; MAX_DEPEND_NODES],
    depend_count      : usize,
}

impl NodeTriggerSig {
    pub fn new() -> Self {
        Self {
            hold_node    : None,
            int_reset    : None,
            depend_nodes : [None; MAX_DEPEND_NODES],
            depend_count : 0,
        }
    }

    pub fn push_depend_node(&mut self, srci: NcpIdent, condi: Option<HcpIdent>) {
        assert!(self.depend_count < MAX_DEPEND_NODES, "depend_nodes capacity exceeded");
        self.depend_nodes[self.depend_count] = Some((srci, condi));
        self.depend_count += 1;
    }

    pub fn iter_depend_nodes(&self) -> impl Iterator<Item = (NcpIdent, Option<HcpIdent>)> + '_ {
        self.depend_nodes[..self.depend_count].iter().map(|n| n.unwrap())
    }

    pub fn depend_count(&self) -> usize { self.depend_count }
}

/// Mirrors the HCP `HasTriggerSig` trait: every concrete node owns one
/// `NodeTriggerSig` and exposes it through the same get/set/add API surface.
pub trait HasNodeTriggerSig {
    fn get_node_triggers    (&self)     -> &NodeTriggerSig;
    fn get_node_triggers_mut(&mut self) -> &mut NodeTriggerSig;

    fn get_hold_node(&self) -> Option<NcpIdent> { self.get_node_triggers().hold_node }
    fn get_int_reset(&self) -> Option<NcpIdent> { self.get_node_triggers().int_reset }

    fn set_hold_node(&mut self, n: NcpIdent) {
        assert!(self.get_node_triggers().hold_node.is_none(), "hold node already set");
        self.get_node_triggers_mut().hold_node = Some(n);
    }
    fn set_int_reset(&mut self, n: NcpIdent) {
        self.get_node_triggers_mut().int_reset = Some(n);
    }

    fn is_there_hold     (&self) -> bool { self.get_node_triggers().hold_node.is_some() }
    fn is_there_int_reset(&self) -> bool { self.get_node_triggers().int_reset.is_some() }

    fn add_depend_node(&mut self, srci: NcpIdent, condi: Option<HcpIdent>) {
        self.get_node_triggers_mut().push_depend_node(srci, condi);
    }
}

// ---- NcpNode trait ----------------------------------------------------------

/// Trait every concrete node implements.  Equivalent to the C++ `Node` virtual
/// surface, with the per-node-base data inlined into each concrete struct.
pub trait NcpNode: HasNodeTriggerSig {
    // ---- ident / clock-mode accessors --------------------------------------
    fn get_ncp_ident    (&self)     -> &NcpIdent;
    fn get_ncp_ident_mut(&mut self) -> &mut NcpIdent;
    fn get_clock_mode   (&self)     -> ClockMode;
    fn set_clock_mode   (&mut self, cm: ClockMode);

    fn get_node_type(&self) -> NodeType { self.get_ncp_ident().get_node_type() }

    // ---- pure virtual ------------------------------------------------------
    fn assign        (&mut self, arena: &mut ModelArena);
    fn get_cycle_used(&self) -> i32;

    // ---- virtual with defaults --------------------------------------------
    fn get_exit_opr       (&self) -> Option<HcpIdent> { None }
    fn get_state_operating(&self) -> Option<HcpIdent> { None }
    fn dry_assign(&mut self, _arena: &mut ModelArena) { panic!("dry_assign: not implemented") }
    fn is_state_full_node(&self) -> bool { true }

    fn get_md_ident_val(&self) -> String {
        format!("{} @ {:p}", self.get_ncp_ident().get_node_type(), self as *const _ as *const ())
    }

    // ---- gating helpers (logic combinators) -------------------------------

    /// `raw_exit & ~int_reset.exit_opr` if this node has an interrupt-reset.
    fn bind_with_rst_output_if_reset(&self, arena: &mut ModelArena, raw_exit: HcpIdent) -> HcpIdent {
        let Some(int_rst_id) = self.get_int_reset() else { return raw_exit };
        let int_exit = arena.get_node_exit_opr(&int_rst_id)
            .expect("int_reset must have exit_opr");
        let inv = arena.make_expression("node_int_inv", LogicOp::BitwiseInvr, int_exit, int_exit, None, None);
        arena.make_expression("node_bind_rst", LogicOp::BitwiseAnd, raw_exit, inv, None, None)
    }

    /// `raw_exit & ~hold.exit_opr` if a hold node is set.
    fn bind_with_hold_if_hold(&self, arena: &mut ModelArena, raw_exit: HcpIdent) -> HcpIdent {
        let Some(hold_id) = self.get_hold_node() else { return raw_exit };
        let hold_exit = arena.get_node_exit_opr(&hold_id)
            .expect("hold_node must have exit_opr");
        let inv = arena.make_expression("node_hold_inv", LogicOp::BitwiseInvr, hold_exit, hold_exit, None, None);
        arena.make_expression("node_bind_hold", LogicOp::BitwiseAnd, raw_exit, inv, None, None)
    }
}

// ---- Free-standing logic-combinator helpers ---------------------------------
//
// Equivalent to C++ `Node::addLogic` / `addLogicWithOutput`.  Provided as free
// functions because they don't need to read any node state.

pub fn add_logic(model_ar: &mut ModelArena, des_logic: &mut Option<HcpIdent>, opr1: HcpIdent, op: LogicOp) {
    assert!(op == LogicOp::BitwiseAnd || op == LogicOp::BitwiseOr);
    if des_logic.is_none() {
        *des_logic = Some(opr1);
    } else {
        let lhs = des_logic.expect("checked above");
        let expr = model_ar.make_expression("node_logic_expr", op, lhs, opr1, None, None);
        *des_logic = Some(expr);
    }
}

pub fn add_logic_with_output(
    model_ar: &mut ModelArena,
    opr1    : Option<HcpIdent>,
    opr2    : Option<HcpIdent>,
    op      : LogicOp,
) -> Option<HcpIdent> {
    assert!(op == LogicOp::BitwiseAnd || op == LogicOp::BitwiseOr);
    match (opr1, opr2) {
        (None,    None   ) => None,
        (Some(a), None   ) => Some(a),
        (None,    Some(b)) => Some(b),
        (Some(a), Some(b)) => Some(model_ar.make_expression("node_logic_expr", op, a, b, None, None)),
    }
}
