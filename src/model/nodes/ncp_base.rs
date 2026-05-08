use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::nodes::ncp_ident::{NcpIdent, NodeType};
use crate::model::model_arena::ModelArena;

pub use crate::model::nodes::node_trigger::{HasNodeTriggerSig, NodeTrigger};

pub const NODE_CYCLE_USED_UNKNOWN: i32 = -1;
pub const IN_CONSIST_CYCLE_USED  : i32 = -2;

// ---- NcpNode trait ----------------------------------------------------------

/// Trait every concrete node implements.  Equivalent to the C++ `Node` virtual
/// surface, with the per-node-base data inlined into each concrete struct.
pub trait NcpNode: HasNodeTriggerSig {
    // ---- ident / clock-mode accessors --------------------------------------
    fn get_ncp_ident    (&self)     -> NcpIdent;
    fn get_clock_mode   (&self)     -> ClockMode;
    fn set_clock_mode   (&mut self, cm: ClockMode);

    fn get_node_type(&self) -> NodeType { self.get_ncp_ident().get_node_type() }

    // ---- pure virtual ------------------------------------------------------
    fn assign        (&mut self, arena: &mut ModelArena);

    // ---- virtual with defaults --------------------------------------------
    fn get_exit_opr       (&self) -> HcpIdent { panic!("get_exit_opr: not implemented") }
    fn get_state_operating(&self) -> HcpIdent { panic!("get_state_operating: not implemented") }
    fn get_cycle_used     (&self) -> i32      { NODE_CYCLE_USED_UNKNOWN }
    fn dry_assign(&mut self, _arena: &mut ModelArena) { panic!("dry_assign: not implemented") }
    fn is_state_full_node(&self) -> bool { true }

    fn get_md_ident_val(&self) -> String {
        format!("{} @ {:p}", self.get_ncp_ident().get_node_type(), self as *const _ as *const ())
    }

    // ---- gating helpers (logic combinators) -------------------------------

    /// `raw_exit & ~int_reset.exit_opr` if this node has an interrupt-reset.
    fn bind_with_rst_output_if_reset(&self, arena: &mut ModelArena, raw_exit: HcpIdent) -> HcpIdent {
        let Some(int_rst_id) = self.get_int_reset_node() else { return raw_exit };
        let int_exit = arena.get_node_exit_opr(&int_rst_id);
        let inv = arena.make_expression("node_int_inv", LogicOp::BitwiseInvr, int_exit, HcpIdent::default(), None, None);
        arena.make_expression("node_bind_rst", LogicOp::BitwiseAnd, raw_exit, inv, None, None)
    }

    /// `raw_exit & ~hold.exit_opr` if a hold node is set.
    fn bind_with_hold_if_hold(&self, arena: &mut ModelArena, raw_exit: HcpIdent) -> HcpIdent {
        let Some(hold_id) = self.get_hold_node()
        else {
            return raw_exit
        };

        let hold_exit = arena.get_node_exit_opr(&hold_id);
        let inv = arena.make_expression("node_hold_inv", LogicOp::BitwiseInvr, hold_exit, HcpIdent::default(), None, None);
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
