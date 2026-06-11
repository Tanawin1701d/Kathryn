use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::slice::Slice;
use crate::model::hw_component::sp_reg::trigger_sig::HasTriggerSig;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_base::{
    HasNodeTriggerSig, NcpNode, NodeTrigger,
    IN_CONSIST_CYCLE_USED,
};
use crate::model::nodes::ncp_ident::{NcpIdent, NodeType};

// ---- WaitCondNode -----------------------------------------------------------

pub struct WaitCondNode {
    ident            : NcpIdent,
    triggers         : NodeTrigger,
    cond_wait_reg_i  : HcpIdent,
    raw_state_op_i   : Option<HcpIdent>,
    bound_exit_i     : Option<HcpIdent>,
}

impl Default for WaitCondNode {
    fn default() -> Self {
        Self {
            ident          : NcpIdent::new(NodeType::WaitCond, false, ""),
            triggers       : NodeTrigger::new(),
            cond_wait_reg_i: HcpIdent::default(),
            raw_state_op_i : None,
            bound_exit_i   : None,
        }
    }
}

impl WaitCondNode {
    pub fn new(is_user_com: bool, name: &str, wait_cond: HcpIdent, wait_cond_sl: Slice, arena: &mut ModelArena) -> Self {
        let cond_wait_reg_i = arena.make_cond_wait_state_reg(&format!("{}_CW", name), wait_cond, wait_cond_sl);
        Self {
            ident          : NcpIdent::new(NodeType::WaitCond, is_user_com, name),
            triggers       : NodeTrigger::new(),
            cond_wait_reg_i,
            raw_state_op_i : None,
            bound_exit_i   : None,
        }
    }

    pub fn get_cond_wait_reg_i(&self) -> HcpIdent { self.cond_wait_reg_i }
}

impl HasNodeTriggerSig for WaitCondNode {
    fn get_node_triggers    (&self)     -> &NodeTrigger { &self.triggers     }
    fn get_node_triggers_mut(&mut self) -> &mut NodeTrigger { &mut self.triggers }
}

impl NcpNode for WaitCondNode {
    fn get_ncp_ident  (&self) -> NcpIdent  { self.ident }
    fn get_clock_mode (&self) -> ClockMode { ClockMode::PosEdge }

    fn assign_prelim(&mut self, arena: &mut ModelArena) {
        let mut cw = arena.take_cond_wait_reg(self.cond_wait_reg_i);
        cw.build_support_signal(arena);
        let end_expr = cw.generate_end_expr();
        arena.replace_back_cond_wait_reg(cw);

        let bound_rst = self.bind_with_rst_output_if_reset(arena, end_expr);
        let bound_all = self.bind_with_hold_if_hold       (arena, bound_rst);
        self.raw_state_op_i = Some(end_expr);
        self.bound_exit_i   = Some(bound_all);
    }

    fn assign_final(&mut self, arena: &mut ModelArena) {
        assert!(self.triggers.depend_count() > 0, "WaitCondNode requires at least one depend node");
        let sig = self.to_trigger_sig(arena);
        let mut cw = arena.take_cond_wait_reg(self.cond_wait_reg_i);
        cw.set_triggers(sig);
        cw.build_update_event(arena);
        arena.replace_back_cond_wait_reg(cw);
    }

    fn get_exit_opr  (&self) -> HcpIdent { self.bound_exit_i.expect("WaitCondNode::get_exit_opr: bound_exit_i not set — call assign_prelim first") }
    fn get_state_operating(&self) -> HcpIdent { self.raw_state_op_i.expect("WaitCondNode::get_state_operating: raw_state_op_i not set — call assign_prelim first") }
    fn get_cycle_used(&self) -> i32 { IN_CONSIST_CYCLE_USED }

    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_wait_cond_node(*self);
    }
}

impl Identifiable for WaitCondNode {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
}

// ---- WaitCycleNode ----------------------------------------------------------

pub struct WaitCycleNode {
    ident            : NcpIdent,
    triggers         : NodeTrigger,
    cycle_wait_reg_i : HcpIdent,
    cycle            : Option<i32>,
    end_expr_i       : Option<HcpIdent>,
    bound_exit_i     : Option<HcpIdent>,
}

impl Default for WaitCycleNode {
    fn default() -> Self {
        Self {
            ident           : NcpIdent::new(NodeType::WaitCycle, false, ""),
            triggers        : NodeTrigger::new(),
            cycle_wait_reg_i: HcpIdent::default(),
            cycle           : None,
            end_expr_i      : None,
            bound_exit_i    : None,
        }
    }
}

impl WaitCycleNode {
    pub fn new_with_cycle(is_user_com: bool, name: &str, cycle: i32, arena: &mut ModelArena) -> Self {
        assert!(cycle > 0);
        let reg_i = arena.make_cycle_wait_state_reg(&format!("{}_CY", name), cycle);
        Self {
            ident           : NcpIdent::new(NodeType::WaitCycle, is_user_com, name),
            triggers        : NodeTrigger::new(),
            cycle_wait_reg_i: reg_i,
            cycle           : Some(cycle),
            end_expr_i      : None,
            bound_exit_i    : None,
        }
    }

    pub fn new_with_expr(is_user_com: bool, name: &str, cnt_bit_sz: i32, end_cnt_i: HcpIdent, arena: &mut ModelArena) -> Self {
        let reg_i = arena.make_cycle_wait_state_reg_with_expr(&format!("{}_CY", name), cnt_bit_sz, end_cnt_i);
        Self {
            ident           : NcpIdent::new(NodeType::WaitCycle, is_user_com, name),
            triggers        : NodeTrigger::new(),
            cycle_wait_reg_i: reg_i,
            cycle           : None,
            end_expr_i      : None,
            bound_exit_i    : None,
        }
    }

    pub fn get_cycle_wait_reg_i(&self) -> HcpIdent { self.cycle_wait_reg_i }
}

impl HasNodeTriggerSig for WaitCycleNode {
    fn get_node_triggers    (&self)     -> &NodeTrigger { &self.triggers     }
    fn get_node_triggers_mut(&mut self) -> &mut NodeTrigger { &mut self.triggers }
}

impl NcpNode for WaitCycleNode {
    fn get_ncp_ident  (&self) -> NcpIdent  { self.ident }
    fn get_clock_mode (&self) -> ClockMode { ClockMode::PosEdge }

    fn assign_prelim(&mut self, arena: &mut ModelArena) {
        let mut cw = arena.take_cycle_wait_reg(self.cycle_wait_reg_i);
        cw.build_support_signal(arena);
        let end_expr = cw.generate_end_expr();
        arena.replace_back_cycle_wait_reg(cw);

        let bound_rst = self.bind_with_rst_output_if_reset(arena, end_expr);
        let bound_all = self.bind_with_hold_if_hold       (arena, bound_rst);
        self.end_expr_i   = Some(end_expr);
        self.bound_exit_i = Some(bound_all);
    }

    fn assign_final(&mut self, arena: &mut ModelArena) {
        let sig = self.to_trigger_sig(arena);
        let mut cw = arena.take_cycle_wait_reg(self.cycle_wait_reg_i);
        cw.set_triggers(sig);
        cw.build_update_event(arena);
        arena.replace_back_cycle_wait_reg(cw);
    }

    fn get_exit_opr  (&self) -> HcpIdent { self.bound_exit_i.expect("WaitCycleNode::get_exit_opr: bound_exit_i not set — call assign_prelim first") }
    fn get_cycle_used(&self) -> i32 {
        if self.get_node_triggers().is_unpred_cycle_usage() {
            IN_CONSIST_CYCLE_USED
        } else {
            self.cycle.unwrap_or(IN_CONSIST_CYCLE_USED)
        }
    }

    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) {
        arena.replace_back_wait_cycle_node(*self);
    }
}

impl Identifiable for WaitCycleNode {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
}
