use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::hw_component::sp_reg::trigger_sig::HasTriggerSig;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_base::{HasNodeTriggerSig, NcpNode, NodeTriggerSig};
use crate::model::nodes::ncp_ident::{NcpIdent, NodeType};

/// Single-shot start node: a 1-bit StateReg raised by the system reset signal.
/// `exit_expr_i` is `state == upState`.
pub struct StartNode {
    ident       : NcpIdent,
    clk_mode    : ClockMode,
    triggers    : NodeTriggerSig,
    rst_sig_i   : HcpIdent,
    state_reg_i : HcpIdent,
    up_state_i  : HcpIdent,
    exit_expr_i : HcpIdent,
}

impl Default for StartNode {
    fn default() -> Self {
        Self {
            ident      : NcpIdent::new(NodeType::Start, false, ""),
            clk_mode   : ClockMode::PosEdge,
            triggers   : NodeTriggerSig::new(),
            rst_sig_i  : HcpIdent::default(),
            state_reg_i: HcpIdent::default(),
            up_state_i : HcpIdent::default(),
            exit_expr_i: HcpIdent::default(),
        }
    }
}

impl StartNode {
    pub fn new(is_user_com: bool, name: &str, rst_sig_i: HcpIdent, arena: &mut ModelArena) -> Self {
        let state_reg_i = arena.make_state_reg(&format!("{}_START_STATE", name));
        let up_state_i  = arena.make_val(&format!("{}_START_UP", name), 1, 1);
        let exit_expr_i = arena.make_expression(
            &format!("{}_START_EXIT", name), LogicOp::RelationEq, state_reg_i, up_state_i,
            None, None,
        );
        Self {
            ident      : NcpIdent::new(NodeType::Start, is_user_com, name),
            clk_mode   : ClockMode::PosEdge,
            triggers   : NodeTriggerSig::new(),
            rst_sig_i, state_reg_i, up_state_i, exit_expr_i,
        }
    }

    pub fn get_state_reg_i(&self) -> HcpIdent { self.state_reg_i }
    pub fn get_up_state_i (&self) -> HcpIdent { self.up_state_i }
}

impl HasNodeTriggerSig for StartNode {
    fn get_node_triggers    (&self)     -> &NodeTriggerSig     { &self.triggers     }
    fn get_node_triggers_mut(&mut self) -> &mut NodeTriggerSig { &mut self.triggers }
}

impl NcpNode for StartNode {
    fn get_ncp_ident    (&self)     -> &NcpIdent     { &self.ident }
    fn get_ncp_ident_mut(&mut self) -> &mut NcpIdent { &mut self.ident }
    fn get_clock_mode   (&self)     -> ClockMode     { self.clk_mode }
    fn set_clock_mode   (&mut self, cm: ClockMode)   { self.clk_mode = cm; }

    fn assign(&mut self, arena: &mut ModelArena) {
        let state_h = *self.state_reg_i.get_arena_handle();
        let mut sr = arena.take_state_reg(state_h);
        sr.add_depend_node(self.rst_sig_i, None);
        sr.build_update_event(arena);
        arena.replace_back_state_reg(state_h, sr);
    }

    fn get_exit_opr  (&self) -> Option<HcpIdent> { Some(self.exit_expr_i) }
    fn get_cycle_used(&self) -> i32              { 1 }
}

impl Identifiable for StartNode {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}
