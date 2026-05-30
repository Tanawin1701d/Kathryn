use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::sp_reg::trigger_sig::HasTriggerSig;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_base::{HasNodeTriggerSig, NcpNode, NodeTrigger};
use crate::model::nodes::ncp_ident::{NcpIdent, NodeType};

/// Cycle-counting node: wraps a `CntReg` that wraps once `last_loop_cnt` is
/// reached.  `get_exit_opr` returns the counter's `at_last` expression after
/// `assign` has been invoked.
pub struct CounterNode {
    ident      : NcpIdent,
    triggers   : NodeTrigger,
    cnt_reg_i  : HcpIdent,
    last_loop  : i32,
    end_expr_i : Option<HcpIdent>,
}

impl Default for CounterNode {
    fn default() -> Self {
        Self {
            ident     : NcpIdent::new(NodeType::Counter, false, ""),
            triggers  : NodeTrigger::new(),
            cnt_reg_i : HcpIdent::default(),
            last_loop : 1,
            end_expr_i: None,
        }
    }
}

impl CounterNode {
    pub fn new(is_user_com: bool, name: &str, last_loop_cnt: i32, arena: &mut ModelArena) -> Self {
        assert!(last_loop_cnt > 0);
        let cnt_reg_i = arena.make_cnt_reg(&format!("{}_CNT", name), 1, last_loop_cnt);
        Self {
            ident     : NcpIdent::new(NodeType::Counter, is_user_com, name),
            triggers  : NodeTrigger::new(),
            cnt_reg_i,
            last_loop : last_loop_cnt,
            end_expr_i: None,
        }
    }

    pub fn get_cnt_reg_i(&self) -> HcpIdent { self.cnt_reg_i }
}

impl HasNodeTriggerSig for CounterNode {
    fn get_node_triggers    (&self)     -> &NodeTrigger { &self.triggers     }
    fn get_node_triggers_mut(&mut self) -> &mut NodeTrigger { &mut self.triggers }
}

impl NcpNode for CounterNode {
    fn get_ncp_ident  (&self) -> NcpIdent  { self.ident }
    fn get_clock_mode (&self) -> ClockMode { ClockMode::PosEdge }

    fn assign(&mut self, arena: &mut ModelArena) {
        let sig = self.to_trigger_sig(arena);
        let mut cr = arena.take_cnt_reg(self.cnt_reg_i);
        cr.set_triggers(sig);

        cr.build_support_signal(arena);
        cr.build_update_event(arena);
        let end_expr = cr.generate_end_expr();
        arena.replace_back_cnt_reg(cr);
        self.end_expr_i = Some(end_expr);
    }

    fn get_exit_opr  (&self) -> HcpIdent { self.end_expr_i.unwrap_or_default() }
    fn get_cycle_used(&self) -> i32      { self.last_loop }

    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_counter_node(*self);
    }
}

impl Identifiable for CounterNode {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}
