use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::hw_component::sp_reg::trigger_sig::HasTriggerSig;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_base::{HasNodeTriggerSig, NcpNode, NodeTrigger};
use crate::model::nodes::ncp_ident::{NcpIdent, NodeType};

pub struct SynNode {
    ident           : NcpIdent,
    triggers        : NodeTrigger,
    sync_reg_i      : HcpIdent,
    syn_size        : i32,
    end_expr_i      : Option<HcpIdent>,
    bound_exit_i    : Option<HcpIdent>,
}

impl Default for SynNode {
    fn default() -> Self {
        Self {
            ident          : NcpIdent::new(NodeType::Syn, false, ""),
            triggers       : NodeTrigger::new(),
            sync_reg_i     : HcpIdent::default(),
            syn_size       : 1,
            end_expr_i     : None,
            bound_exit_i   : None,
        }
    }
}

impl SynNode {
    pub fn new(is_user_com: bool, name: &str, syn_size: i32, arena: &mut ModelArena) -> Self {
        assert!(syn_size > 0);
        let sync_reg_i = arena.make_sync_reg(&format!("{}_SY", name), syn_size);
        Self {
            ident          : NcpIdent::new(NodeType::Syn, is_user_com, name),
            triggers       : NodeTrigger::new(),
            sync_reg_i,
            syn_size,
            end_expr_i     : None,
            bound_exit_i   : None,
        }
    }

    pub fn get_sync_reg_i(&self) -> HcpIdent { self.sync_reg_i }
    pub fn get_syn_size  (&self) -> i32      { self.syn_size }
}

impl HasNodeTriggerSig for SynNode {
    fn get_node_triggers    (&self)     -> &NodeTrigger { &self.triggers     }
    fn get_node_triggers_mut(&mut self) -> &mut NodeTrigger { &mut self.triggers }
}

impl NcpNode for SynNode {
    fn get_ncp_ident  (&self) -> NcpIdent  { self.ident }
    fn get_clock_mode (&self) -> ClockMode { ClockMode::PosEdge }

    fn assign_prelim(&mut self, _arena: &mut ModelArena) {}

    fn assign_final(&mut self, arena: &mut ModelArena) {

        let sig = self.to_trigger_sig(arena);

        let mut sy = arena.take_sync_reg(self.sync_reg_i);
        sy.set_triggers(sig);
        sy.build_support_signal(arena);
        sy.build_update_event(arena);
        let end_expr = sy.get_end_expr_i().expect("end_expr after build");
        arena.replace_back_sync_reg(sy);

        let bound_reset = self.bind_with_rst_output_if_reset(arena, end_expr);
        let bound_all = self.bind_with_hold_if_hold(arena, bound_reset);
        self.end_expr_i   = Some(end_expr);
        self.bound_exit_i = Some(bound_all);
    }

    fn get_exit_opr      (&self) -> HcpIdent { self.bound_exit_i.unwrap() }
    fn get_cycle_used    (&self) -> i32      { 0 }
    fn is_state_full_node(&self) -> bool     { false }

    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_syn_node(*self);
    }
}

impl Identifiable for SynNode {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}
