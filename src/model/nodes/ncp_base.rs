use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::hw_component::sp_reg::trigger_sig::TriggerSig;
use crate::model::nodes::ncp_ident::{NcpIdent, NodeType};
use crate::model::model_arena::ModelArena;

pub use crate::model::nodes::node_trigger::{HasNodeTriggerSig, NodeTrigger};

pub const IN_CONSIST_CYCLE_USED: i32 = -1;

// ---- NcpNode trait ----------------------------------------------------------

/// Trait every concrete node implements.  Equivalent to the C++ `Node` virtual
/// surface, with the per-node-base data inlined into each concrete struct.
pub trait NcpNode: HasNodeTriggerSig {
    // ---- ident / clock-mode accessors --------------------------------------
    fn get_ncp_ident  (&self) -> NcpIdent;
    fn get_clock_mode (&self) -> ClockMode;

    fn get_node_type(&self) -> NodeType { self.get_ncp_ident().get_node_type() }

    // ---- pure virtual ------------------------------------------------------
    fn assign_prelim(&mut self, arena: &mut ModelArena); /// the assign the signal only enough for graph connection, it should be use after fill_ext_node is assigned
    fn assign_final (&mut self, arena: &mut ModelArena); /// the assign when all internal register's dependency is connected
    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena);

    // ---- virtual with defaults --------------------------------------------
    fn get_exit_opr       (&self) -> HcpIdent { panic!("get_exit_opr: not implemented") }
    fn get_state_operating(&self) -> HcpIdent { panic!("get_state_operating: not implemented") }
    fn get_cycle_used     (&self) -> i32      { IN_CONSIST_CYCLE_USED }
    fn dry_assign(&mut self, _arena: &mut ModelArena) { panic!("dry_assign: not implemented") }
    fn is_state_full_node(&self) -> bool { true }

    fn get_md_ident_val(&self) -> String {
        format!("{} @ {:p}", self.get_ncp_ident().get_node_type(), self as *const _ as *const ())
    }

    // ---- TriggerSig builders ----------------------------------------------

    /// Resolve a referenced node's exit-opr safely.  If the target is *this*
    /// node, the arena slot is currently taken (self-loopback) so calling
    /// `arena.get_node_exit_opr` would panic — fall back to `self.get_exit_opr()`.
    fn resolve_node_exit_opr(&self, arena: &ModelArena, n: NcpIdent) -> HcpIdent {
        if n == self.get_ncp_ident() { self.get_exit_opr() }
        else                         { arena.get_node_exit_opr(&n) }
    }

    /// Initialise hold / int-reset / master-reset / clk fields of a TriggerSig
    /// from this node's NodeTrigger.  int-start is intentionally excluded —
    /// call `init_int_start_sig` explicitly for nodes that need it.
    fn init_ctrl_sigs(&self, sig: &mut TriggerSig, arena: &ModelArena) {
        let nt = self.get_node_triggers();
        sig.hold_sig_i    = nt.hold_node_i     .map(|n| self.resolve_node_exit_opr(arena, n));
        sig.int_rst_sig_i = nt.int_reset_node_i.map(|n| self.resolve_node_exit_opr(arena, n));
        sig.mrst_sig_i    = nt.mrst_node_i     .map(|n| self.resolve_node_exit_opr(arena, n));
        sig.clk_sig_i     = nt.clk_node_i      .map(|n| self.resolve_node_exit_opr(arena, n));
    }

    /// Opt-in initialisation of int_start_sig_i for nodes that use it.
    fn init_int_start_sig(&self, sig: &mut TriggerSig, arena: &ModelArena) {
        sig.int_start_sig_i = self.get_node_triggers().int_start_node_i
            .map(|n| self.resolve_node_exit_opr(arena, n));
    }

    fn to_trigger_sig(&self, arena: &ModelArena) -> TriggerSig {
        let mut sig = TriggerSig::new();
        self.init_ctrl_sigs    (&mut sig, arena);
        self.init_int_start_sig(&mut sig, arena);
        for (srci, condi) in self.get_node_triggers().iter_depend_nodes() {
            sig.push_depend_node(self.resolve_node_exit_opr(arena, srci), condi);
        }
        sig
    }

    // ---- gating helpers (logic combinators) -------------------------------

    /// `raw_exit & ~int_reset.exit_opr` if this node has an interrupt-reset.
    fn bind_with_rst_output_if_reset(&self, arena: &mut ModelArena, raw_exit: HcpIdent) -> HcpIdent {
        let Some(int_rst_id) = self.get_int_reset_node() else { return raw_exit };
        let int_exit = arena.get_node_exit_opr(&int_rst_id);
        let inv = arena.make_expression_single(false, "node_int_inv", LogicOp::BitwiseInvr, int_exit, None);
        arena.make_expression(false, "node_bind_rst", LogicOp::BitwiseAnd, raw_exit, inv, None, None)
    }

    /// `raw_exit & ~hold.exit_opr` if a hold node is set.
    fn bind_with_hold_if_hold(&self, arena: &mut ModelArena, raw_exit: HcpIdent) -> HcpIdent {
        let Some(hold_id) = self.get_hold_node()
        else {
            return raw_exit
        };

        let hold_exit = arena.get_node_exit_opr(&hold_id);
        let inv = arena.make_expression_single(false, "node_hold_inv", LogicOp::BitwiseInvr, hold_exit, None);
        arena.make_expression(false, "node_bind_hold", LogicOp::BitwiseAnd, raw_exit, inv, None, None)
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
        let expr = model_ar.make_expression(false, "node_logic_expr", op, lhs, opr1, None, None);
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
        (Some(a), Some(b)) => Some(model_ar.make_expression(false, "node_logic_expr", op, a, b, None, None)),
    }
}
