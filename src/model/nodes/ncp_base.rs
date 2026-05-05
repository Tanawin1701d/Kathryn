use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::hw_component::sp_reg::trigger_sig::TriggerSig;
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

#[derive(Clone, Debug, PartialEq, Eq, Default)]
pub struct NodeTrigger {
    pub hold_node_i      : Option<NcpIdent>,
    pub int_reset_node_i : Option<NcpIdent>,
    pub int_start_node_i : Option<NcpIdent>,
    pub mrst_node_i      : Option<NcpIdent>,
        depend_nodes     : Vec<(NcpIdent, Option<HcpIdent>)>,
}

impl NodeTrigger {
    pub fn new() -> Self {
        Self {
            hold_node_i      : None,
            int_reset_node_i : None,
            int_start_node_i : None,
            mrst_node_i      : None,
            depend_nodes     : Vec::new(),
        }
    }

    pub fn push_depend_node(&mut self, srci: NcpIdent, condi: Option<HcpIdent>) {
        assert!(self.depend_nodes.len() < MAX_DEPEND_NODES, "depend_nodes capacity exceeded");
        self.depend_nodes.push((srci, condi));
    }

    pub fn iter_depend_nodes(&self) -> impl Iterator<Item = (NcpIdent, Option<HcpIdent>)> + '_ {
        self.depend_nodes.iter().copied()
    }

    pub fn depend_count(&self) -> usize { self.depend_nodes.len() }

    pub fn to_trigger_sig(&self, arena: &ModelArena) -> TriggerSig {
        let mut sig = TriggerSig::new();
        sig.hold_sig_i      = self.hold_node_i     .map(|n| arena.get_node_exit_opr(&n));
        sig.int_rst_sig_i   = self.int_reset_node_i.map(|n| arena.get_node_exit_opr(&n));
        sig.int_start_sig_i = self.int_start_node_i.map(|n| arena.get_node_exit_opr(&n));
        sig.mrst_sig_i      = self.mrst_node_i     .map(|n| arena.get_node_exit_opr(&n));
        for (srci, condi) in self.iter_depend_nodes() {
            sig.push_depend_node(arena.get_node_exit_opr(&srci), condi);
        }
        sig
    }





}

pub trait HasNodeTriggerSig {
    fn get_node_triggers    (&self)     -> &NodeTrigger;
    fn get_node_triggers_mut(&mut self) -> &mut NodeTrigger;

    fn get_hold_node     (&self) -> Option<NcpIdent> { self.get_node_triggers().hold_node_i }
    fn get_int_reset_node(&self) -> Option<NcpIdent> { self.get_node_triggers().int_reset_node_i }
    fn get_int_start_node(&self) -> Option<NcpIdent> { self.get_node_triggers().int_start_node_i }
    fn get_mrst_node     (&self) -> Option<NcpIdent> { self.get_node_triggers().mrst_node_i }

    fn set_hold_node     (&mut self, n: NcpIdent) { self.get_node_triggers_mut().hold_node_i      = Some(n); }
    fn set_int_reset_node(&mut self, n: NcpIdent) { self.get_node_triggers_mut().int_reset_node_i = Some(n); }
    fn set_int_start_node(&mut self, n: NcpIdent) { self.get_node_triggers_mut().int_start_node_i = Some(n); }
    fn set_mrst_node     (&mut self, n: NcpIdent) { self.get_node_triggers_mut().mrst_node_i      = Some(n); }

    fn add_depend_node(&mut self, srci: NcpIdent, condi: Option<HcpIdent>) {
        self.get_node_triggers_mut().push_depend_node(srci, condi);
    }
}

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
    fn get_exit_opr       (&self) -> HcpIdent { HcpIdent::default() }
    fn get_state_operating(&self) -> HcpIdent { HcpIdent::default() }
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
        let Some(hold_id) = self.get_hold_node() else { return raw_exit };
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
