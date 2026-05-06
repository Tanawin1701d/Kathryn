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
        let init_expr = arena.make_expression_empty("pseudo_init_expr", bit_width);
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
    fn get_ncp_ident    (&self)     -> NcpIdent     { self.ident }
    fn get_clock_mode   (&self)     -> ClockMode     { ClockMode::ClkFree }
    fn set_clock_mode   (&mut self, _cm: ClockMode)  { panic!("PseudoNode: clock mode is fixed to ClkFree") }

    fn assign(&mut self, arena: &mut ModelArena) {
        let depend_count = self.triggers.depend_count();
        assert!(depend_count > 0, "PseudoNode requires at least one depend node");

        let dep_list: Vec<(NcpIdent, Option<HcpIdent>)> =
            self.triggers.iter_depend_nodes().collect();

        let mut final_opr: Option<HcpIdent> = None;
        for (src_node, condition) in dep_list {
            let src_exit = arena.get_node_exit_opr(&src_node);
            let mut opr_per_src = Some(src_exit);
            if let Some(c) = condition {
                add_logic(arena, &mut opr_per_src, c, LogicOp::BitwiseAnd);
            }
            let opr = opr_per_src.expect("checked above");
            add_logic(arena, &mut final_opr, opr, self.join_op);
        }
        let src = final_opr.expect("depend_count > 0 asserted above");
        arena.get_expression_mut(self.exit_expr_i).assign_operand(src, Slice::new(0, self.bit_width));
    }

    fn get_exit_opr     (&self) -> HcpIdent { self.exit_expr_i }
    fn is_state_full_node(&self) -> bool    { false }
}

impl Identifiable for PseudoNode {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}

// ---- DummyNode --------------------------------------------------------------

/// Wraps a constant `Val` so it can stand in as a node.
pub struct DummyNode {
    ident    : NcpIdent,
    clk_mode : ClockMode,
    triggers : NodeTrigger,
    value_i  : HcpIdent,
}

impl Default for DummyNode {
    fn default() -> Self {
        Self {
            ident   : NcpIdent::new(NodeType::Dummy, false, ""),
            clk_mode: ClockMode::ClkFree,
            triggers: NodeTrigger::new(),
            value_i : HcpIdent::default(),
        }
    }
}

impl DummyNode {
    pub fn new(is_user_com: bool, name: &str, value_i: HcpIdent) -> Self {
        Self {
            ident   : NcpIdent::new(NodeType::Dummy, is_user_com, name),
            clk_mode: ClockMode::ClkFree,
            triggers: NodeTrigger::new(),
            value_i,
        }
    }
    pub fn get_value_i(&self) -> HcpIdent { self.value_i }
}

impl HasNodeTriggerSig for DummyNode {
    fn get_node_triggers    (&self)     -> &NodeTrigger { &self.triggers     }
    fn get_node_triggers_mut(&mut self) -> &mut NodeTrigger { &mut self.triggers }
}

impl NcpNode for DummyNode {
    fn get_ncp_ident    (&self)     -> NcpIdent     { self.ident }
    fn get_clock_mode   (&self)     -> ClockMode     { self.clk_mode }
    fn set_clock_mode   (&mut self, cm: ClockMode)   { self.clk_mode = cm; }

    fn assign(&mut self, _arena: &mut ModelArena) {
        assert_eq!(self.triggers.depend_count(), 0, "DummyNode does not accept depend nodes");
    }

    fn get_exit_opr     (&self) -> HcpIdent { self.value_i }
    fn is_state_full_node(&self) -> bool             { false }
}

impl Identifiable for DummyNode {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}

// ---- OprNode ----------------------------------------------------------------

/// Wraps an arbitrary operand (any HCP) so it can be used as a node output.
pub struct OprNode {
    ident    : NcpIdent,
    clk_mode : ClockMode,
    triggers : NodeTrigger,
    value_i  : HcpIdent,
}

impl Default for OprNode {
    fn default() -> Self {
        Self {
            ident   : NcpIdent::new(NodeType::Opr, false, ""),
            clk_mode: ClockMode::ClkFree,
            triggers: NodeTrigger::new(),
            value_i : HcpIdent::default(),
        }
    }
}

impl OprNode {
    pub fn new(is_user_com: bool, name: &str, value_i: HcpIdent) -> Self {
        Self {
            ident   : NcpIdent::new(NodeType::Opr, is_user_com, name),
            clk_mode: ClockMode::ClkFree,
            triggers: NodeTrigger::new(),
            value_i,
        }
    }
    pub fn get_value_i(&self) -> HcpIdent { self.value_i }
}

impl HasNodeTriggerSig for OprNode {
    fn get_node_triggers    (&self)     -> &NodeTrigger { &self.triggers     }
    fn get_node_triggers_mut(&mut self) -> &mut NodeTrigger { &mut self.triggers }
}

impl NcpNode for OprNode {
    fn get_ncp_ident    (&self)     -> NcpIdent     { self.ident }
    fn get_clock_mode   (&self)     -> ClockMode     { self.clk_mode }
    fn set_clock_mode   (&mut self, cm: ClockMode)   { self.clk_mode = cm; }

    fn assign(&mut self, _arena: &mut ModelArena) {
        assert_eq!(self.triggers.depend_count(), 0, "OprNode does not accept depend nodes");
    }

    fn get_exit_opr     (&self) -> HcpIdent { self.value_i }
    fn is_state_full_node(&self) -> bool             { false }
}

impl Identifiable for OprNode {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}
