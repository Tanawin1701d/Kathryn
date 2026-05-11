use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::hw_component::sp_reg::trigger_sig::HasTriggerSig;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_base::{HasNodeTriggerSig, NcpNode, NodeTrigger};
use crate::model::nodes::ncp_ident::{NcpIdent, NodeType};

pub struct StartNode {
    ident       : NcpIdent,
    triggers    : NodeTrigger,
    state_reg_i : HcpIdent,
    rst_sig_i   : HcpIdent,
    exit_expr_i : Option<HcpIdent>,
}

impl Default for StartNode {
    fn default() -> Self {
        Self {
            ident       : NcpIdent::new(NodeType::Start, false, ""),
            triggers    : NodeTrigger::new(),
            state_reg_i : HcpIdent::default(),
            rst_sig_i   : HcpIdent::default(),
            exit_expr_i : None,
        }
    }
}

impl StartNode {
    pub fn new(name: &str, rst_sig_i: HcpIdent, arena: &mut ModelArena) -> Self {
        let state_reg_i = arena.make_state_reg(&format!("{}_ST", name));
        let up_state_i  = arena.make_val("upState", 1, 1);
        let exit_expr_i = arena.make_expression(
            "startExpr", LogicOp::RelationEq, state_reg_i, up_state_i, None, None,
        );
        Self {
            ident       : NcpIdent::new(NodeType::Start, false, name),
            triggers    : NodeTrigger::new(),
            state_reg_i,
            rst_sig_i,
            exit_expr_i : Some(exit_expr_i),
        }
    }
}

impl HasNodeTriggerSig for StartNode {
    fn get_node_triggers    (&self)     -> &NodeTrigger     { &self.triggers     }
    fn get_node_triggers_mut(&mut self) -> &mut NodeTrigger { &mut self.triggers }
}

impl NcpNode for StartNode {
    fn get_ncp_ident  (&self) -> NcpIdent  { self.ident }
    fn get_clock_mode (&self) -> ClockMode { ClockMode::PosEdge }

    fn assign(&mut self, arena: &mut ModelArena) {
        let mut sr = arena.take_state_reg(self.state_reg_i);
        sr.add_depend_node(self.rst_sig_i, None);
        sr.build_update_event(arena);
        arena.replace_back_state_reg(sr);
    }

    fn get_exit_opr  (&self) -> HcpIdent { self.exit_expr_i.unwrap_or_default() }
    fn get_cycle_used(&self) -> i32      { 1 }

    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_start_node(*self);
    }
}

impl Identifiable for StartNode {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}
