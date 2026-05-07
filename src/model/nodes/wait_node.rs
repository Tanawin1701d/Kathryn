use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::sp_reg::trigger_sig::HasTriggerSig;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_base::{
    HasNodeTriggerSig, NcpNode, NodeTrigger,
    IN_CONSIST_CYCLE_USED, NODE_CYCLE_USED_UNKNOWN,
};
use crate::model::nodes::ncp_ident::{NcpIdent, NodeType};

// ---- WaitCondNode -----------------------------------------------------------

pub struct WaitCondNode {
    ident            : NcpIdent,
    clk_mode         : ClockMode,
    triggers         : NodeTrigger,
    cond_wait_reg_i  : HcpIdent,
    end_expr_i       : Option<HcpIdent>,
    raw_state_op_i   : Option<HcpIdent>,
    bound_exit_i     : Option<HcpIdent>,
}

impl Default for WaitCondNode {
    fn default() -> Self {
        Self {
            ident          : NcpIdent::new(NodeType::WaitCond, false, ""),
            clk_mode       : ClockMode::PosEdge,
            triggers       : NodeTrigger::new(),
            cond_wait_reg_i: HcpIdent::default(),
            end_expr_i     : None,
            raw_state_op_i : None,
            bound_exit_i   : None,
        }
    }
}

impl WaitCondNode {
    pub fn new(is_user_com: bool, name: &str, wait_cond: HcpIdent, clk_mode: ClockMode, arena: &mut ModelArena) -> Self {
        let cond_wait_reg_i = arena.make_cond_wait_state_reg(&format!("{}_CW", name), wait_cond);
        Self {
            ident          : NcpIdent::new(NodeType::WaitCond, is_user_com, name),
            clk_mode,
            triggers       : NodeTrigger::new(),
            cond_wait_reg_i,
            end_expr_i     : None,
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
    fn get_ncp_ident    (&self)     -> NcpIdent     { self.ident }
    fn get_clock_mode   (&self)     -> ClockMode     { self.clk_mode }
    fn set_clock_mode   (&mut self, cm: ClockMode)   { self.clk_mode = cm; }

    fn assign(&mut self, arena: &mut ModelArena) {
        assert!(self.triggers.depend_count() > 0, "WaitCondNode requires at least one depend node");

        let cm = self.clk_mode;

        let dep_list: Vec<(NcpIdent, Option<HcpIdent>)> =
            self.triggers.iter_depend_nodes().collect();
        let int_rst_exit = self.get_int_reset_node()
            .map(|ir| arena.get_node_exit_opr(&ir));

        let mut cw = arena.take_cond_wait_reg(self.cond_wait_reg_i);
        for (src_node, condition) in dep_list {
            let src_exit = arena.get_node_exit_opr(&src_node);
            cw.add_depend_node(src_exit, condition);
        }
        if let Some(rst_exit) = int_rst_exit { cw.set_rst_sig_i(rst_exit); }
        cw.build_support_signal(arena);
        cw.build_update_event(arena);
        let end_expr = cw.generate_end_expr();
        arena.replace_back_cond_wait_reg(cw);

        let bound_rst = self.bind_with_rst_output_if_reset(arena, end_expr);
        let bound_all = self.bind_with_hold_if_hold(arena, bound_rst);
        self.raw_state_op_i = Some(end_expr);
        self.end_expr_i     = Some(end_expr);
        self.bound_exit_i   = Some(bound_all);
    }

    fn get_exit_opr  (&self) -> HcpIdent { self.bound_exit_i.or(self.end_expr_i).unwrap_or_default() }
    fn get_state_operating(&self) -> HcpIdent { self.raw_state_op_i.unwrap_or_default() }
}

impl Identifiable for WaitCondNode {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}

// ---- WaitCycleNode ----------------------------------------------------------

pub struct WaitCycleNode {
    ident            : NcpIdent,
    clk_mode         : ClockMode,
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
            clk_mode        : ClockMode::PosEdge,
            triggers        : NodeTrigger::new(),
            cycle_wait_reg_i: HcpIdent::default(),
            cycle           : None,
            end_expr_i      : None,
            bound_exit_i    : None,
        }
    }
}

impl WaitCycleNode {
    pub fn new_with_cycle(is_user_com: bool, name: &str, cycle: i32, clk_mode: ClockMode, arena: &mut ModelArena) -> Self {
        assert!(cycle > 0);
        let reg_i = arena.make_cycle_wait_state_reg(&format!("{}_CY", name), cycle);
        Self {
            ident           : NcpIdent::new(NodeType::WaitCycle, is_user_com, name),
            clk_mode,
            triggers        : NodeTrigger::new(),
            cycle_wait_reg_i: reg_i,
            cycle           : Some(cycle),
            end_expr_i      : None,
            bound_exit_i    : None,
        }
    }

    pub fn new_with_expr(is_user_com: bool, name: &str, cnt_bit_sz: i32, end_cnt_i: HcpIdent, clk_mode: ClockMode, arena: &mut ModelArena) -> Self {
        let reg_i = arena.make_cycle_wait_state_reg_with_expr(&format!("{}_CY", name), cnt_bit_sz, end_cnt_i);
        Self {
            ident           : NcpIdent::new(NodeType::WaitCycle, is_user_com, name),
            clk_mode,
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
    fn get_ncp_ident    (&self)     -> NcpIdent     { self.ident }
    fn get_clock_mode   (&self)     -> ClockMode     { self.clk_mode }
    fn set_clock_mode   (&mut self, cm: ClockMode)   { self.clk_mode = cm; }

    fn assign(&mut self, arena: &mut ModelArena) {
        let cm = self.clk_mode;

        let dep_list: Vec<(NcpIdent, Option<HcpIdent>)> =
            self.triggers.iter_depend_nodes().collect();
        let hold_sig_i  = self.get_hold_node().map(|h| arena.get_node_exit_opr(&h));
        let int_rst_sig = self.get_int_reset_node().map(|h| arena.get_node_exit_opr(&h));

        let mut cw = arena.take_cycle_wait_reg(self.cycle_wait_reg_i);
        for (src_node, condition) in dep_list {
            let src_exit = arena.get_node_exit_opr(&src_node);
            cw.add_depend_node(src_exit, condition);
        }
        if let Some(hs) = hold_sig_i  { cw.set_hold_sig_i(hs); }
        if let Some(rs) = int_rst_sig { cw.set_rst_sig_i(rs); }

        cw.build_support_signal(arena);
        cw.build_update_event(arena);
        let end_expr = cw.generate_end_expr();
        arena.replace_back_cycle_wait_reg(cw);

        let bound_rst = self.bind_with_rst_output_if_reset(arena, end_expr);
        let bound_all = self.bind_with_hold_if_hold(arena, bound_rst);
        self.end_expr_i   = Some(end_expr);
        self.bound_exit_i = Some(bound_all);
    }

    fn get_exit_opr  (&self) -> HcpIdent { self.bound_exit_i.or(self.end_expr_i).unwrap_or_default() }
}

impl Identifiable for WaitCycleNode {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}
