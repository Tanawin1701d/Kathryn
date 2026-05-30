use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::hw_component::common::slice::Slice;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_base::{
    add_logic, HasNodeTriggerSig, NcpNode, NodeTrigger,
};
use crate::model::nodes::ncp_ident::{NcpIdent, NodeType};

// ---- PseudoNode -------------------------------------------------------------

/// Combinational fan-in node: joins all dependent-node exit operands with
/// `join_op` (BitwiseAnd / BitwiseOr).  The combined operand is materialised
/// in the arena during `assign` and exposed via `get_exit_opr`.
pub struct PseudoNode {
    ident       : NcpIdent,
    triggers    : NodeTrigger,
    bit_width   : i32,
    join_op     : LogicOp,
    exit_expr_i : HcpIdent,
}

impl Default for PseudoNode {
    fn default() -> Self {
        Self {
            ident      : NcpIdent::new(NodeType::Pseudo, false, ""),
            triggers   : NodeTrigger::new(),
            bit_width  : 1,
            join_op    : LogicOp::BitwiseAnd,
            exit_expr_i: HcpIdent::default(),
        }
    }
}

impl PseudoNode {
    pub fn new(is_user_com: bool, name: &str, bit_width: i32, join_op: LogicOp, arena: &mut ModelArena) -> Self {
        assert!(matches!(join_op, LogicOp::BitwiseAnd | LogicOp::BitwiseOr));
        let init_expr = arena.make_expression_empty(false, "pseudo_init_expr", bit_width);
        Self {
            ident      : NcpIdent::new(NodeType::Pseudo, is_user_com, name),
            triggers   : NodeTrigger::new(),
            bit_width,
            join_op,
            exit_expr_i: init_expr,
        }
    }
    pub fn get_bit_width(&self) -> i32     { self.bit_width }
    pub fn get_join_op  (&self) -> LogicOp { self.join_op }
}

impl HasNodeTriggerSig for PseudoNode {
    fn get_node_triggers    (&self)     -> &NodeTrigger { &self.triggers     }
    fn get_node_triggers_mut(&mut self) -> &mut NodeTrigger { &mut self.triggers }
}

impl NcpNode for PseudoNode {
    fn get_ncp_ident  (&self) -> NcpIdent  { self.ident }
    fn get_clock_mode (&self) -> ClockMode { ClockMode::ClkFree }

    fn assign_prelim(&mut self, _arena: &mut ModelArena) {}

    // Deferred to assign_final so every dependent node's exit_opr is already
    // bound (set in their assign_prelim) before we fold it into exit_expr_i.
    fn assign_final(&mut self, arena: &mut ModelArena) {
        let depend_count = self.triggers.depend_count();
        assert!(depend_count > 0, "PseudoNode requires at least one depend node");

        // Snapshot the dep list so we drop the borrow on self.triggers before mutating arena.
        let dep_list: Vec<(NcpIdent, Option<HcpIdent>)> =
            self.triggers.iter_depend_nodes().collect();

        // Fold each dep's (cond-gated) exit_opr into final_opr_i via join_op.
        let mut final_opr_i: Option<HcpIdent> = None;
        for (src_node, condition) in dep_list {
            // resolve_node_exit_opr handles self-loopback safely (slot may be taken).
            let src_exit_i = self.resolve_node_exit_opr(arena, src_node);
            let mut opr_per_src_i = Some(src_exit_i);
            if let Some(c) = condition {
                // AND-gate this dep's exit with the per-edge condition signal.
                add_logic(arena, &mut opr_per_src_i, c, LogicOp::BitwiseAnd);
            }
            add_logic(arena,
                      &mut final_opr_i,
                      opr_per_src_i.expect("checked above"),
                      self.join_op);
        }

        // Materialise the folded operand into the pre-allocated exit_expr placeholder.
        let mut expr = arena.take_expression(self.exit_expr_i);
        expr.assign_operand(final_opr_i.expect("depend_count > 0 asserted above"),
                            Slice::new(0, self.bit_width));
        arena.replace_back_expression(expr);
    }

    fn get_exit_opr     (&self) -> HcpIdent { self.exit_expr_i }
    fn get_cycle_used   (&self) -> i32      { 0 }
    fn is_state_full_node(&self) -> bool    { false }

    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_pseudo_node(*self);
    }
}

impl Identifiable for PseudoNode {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}

// ---- OprNode ----------------------------------------------------------------

/// Wraps an arbitrary operand (any HCP) so it can be used as a node output.
pub struct OprNode {
    ident    : NcpIdent,
    value_i  : HcpIdent,
}

impl Default for OprNode {
    fn default() -> Self {
        Self {
            ident   : NcpIdent::new(NodeType::Opr, false, ""),
            value_i : HcpIdent::default(),
        }
    }
}

impl OprNode {
    pub fn new(is_user_com: bool, name: &str, value_i: HcpIdent) -> Self {
        Self {
            ident   : NcpIdent::new(NodeType::Opr, is_user_com, name),
            value_i,
        }
    }
    pub fn get_value_i(&self) -> HcpIdent { self.value_i }
}

impl HasNodeTriggerSig for OprNode {
    fn get_node_triggers    (&self)     -> &NodeTrigger     { panic!("OprNode has no triggers") }
    fn get_node_triggers_mut(&mut self) -> &mut NodeTrigger { panic!("OprNode has no triggers") }
}

impl NcpNode for OprNode {
    fn get_ncp_ident  (&self) -> NcpIdent  { self.ident }
    fn get_clock_mode (&self) -> ClockMode { ClockMode::ClkFree }

    fn assign_prelim(&mut self, _arena: &mut ModelArena) {}

    fn assign_final(&mut self, _arena: &mut ModelArena) {
        panic!("OprNode: assign() is not implemented");
    }

    fn get_exit_opr     (&self) -> HcpIdent { self.value_i }
    fn get_cycle_used   (&self) -> i32      { 0 }
    fn is_state_full_node(&self) -> bool    { false }

    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_opr_node(*self);
    }
}

impl Identifiable for OprNode {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}
