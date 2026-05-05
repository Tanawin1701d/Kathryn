use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::hw_component::sp_reg::trigger_sig::HasTriggerSig;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_base::{
    HasNodeTriggerSig, NcpNode, NodeTrigger, NODE_CYCLE_USED_UNKNOWN,
};
use crate::model::nodes::ncp_ident::{NcpIdent, NodeType};
use crate::params::MAX_DEPEND_NODES;

// ---- StateNode --------------------------------------------------------------

pub struct StateNode {
    ident           : NcpIdent,
    clk_mode        : ClockMode,
    triggers        : NodeTrigger,
    state_reg_i     : HcpIdent,
    state_op_i      : Option<HcpIdent>,
    bound_exit_i    : Option<HcpIdent>,
    /// Slave AsmNodes (referenced by NcpIdent).  See `assignFromStateNode` in C++.
    slave_asm_nodes : [Option<NcpIdent>; MAX_DEPEND_NODES],
    slave_conds     : [Option<HcpIdent>; MAX_DEPEND_NODES],
    slave_asm_count : usize,
}

impl Default for StateNode {
    fn default() -> Self {
        Self {
            ident          : NcpIdent::new(NodeType::State, false, ""),
            clk_mode       : ClockMode::PosEdge,
            triggers       : NodeTrigger::new(),
            state_reg_i    : HcpIdent::default(),
            state_op_i     : None,
            bound_exit_i   : None,
            slave_asm_nodes: [None; MAX_DEPEND_NODES],
            slave_conds    : [None; MAX_DEPEND_NODES],
            slave_asm_count: 0,
        }
    }
}

impl StateNode {
    pub fn new(is_user_com: bool, name: &str, clk_mode: ClockMode, arena: &mut ModelArena) -> Self {
        let state_reg_i = arena.make_state_reg(&format!("{}_ST", name));
        Self {
            ident          : NcpIdent::new(NodeType::State, is_user_com, name),
            clk_mode,
            triggers       : NodeTrigger::new(),
            state_reg_i,
            state_op_i     : None,
            bound_exit_i   : None,
            slave_asm_nodes: [None; MAX_DEPEND_NODES],
            slave_conds    : [None; MAX_DEPEND_NODES],
            slave_asm_count: 0,
        }
    }

    pub fn add_slave_asm_node(&mut self, asm_node: NcpIdent, cond: Option<HcpIdent>) {
        assert!(self.slave_asm_count < MAX_DEPEND_NODES, "slave asm capacity exceeded");
        self.slave_asm_nodes[self.slave_asm_count] = Some(asm_node);
        self.slave_conds    [self.slave_asm_count] = cond;
        self.slave_asm_count += 1;
    }

    pub fn get_state_reg_i(&self) -> HcpIdent { self.state_reg_i }
}

impl HasNodeTriggerSig for StateNode {
    fn get_node_triggers    (&self)     -> &NodeTrigger { &self.triggers     }
    fn get_node_triggers_mut(&mut self) -> &mut NodeTrigger { &mut self.triggers }
}

impl NcpNode for StateNode {
    fn get_ncp_ident    (&self)     -> NcpIdent     { self.ident }
    fn get_clock_mode   (&self)     -> ClockMode     { self.clk_mode }
    fn set_clock_mode   (&mut self, cm: ClockMode)   { self.clk_mode = cm; }

    fn assign(&mut self, arena: &mut ModelArena) {
        let h = *self.state_reg_i.get_arena_handle();

        let dep_list: Vec<(NcpIdent, Option<HcpIdent>)> =
            self.triggers.iter_depend_nodes().collect();
        let hold_sig    = self.get_hold_node().map(|h| arena.get_node_exit_opr(&h));
        let int_rst_sig = self.get_int_reset_node().map(|h| arena.get_node_exit_opr(&h));

        let mut sr = arena.take_state_reg(h);
        for (src_node, condition) in dep_list {
            let src_exit = arena.get_node_exit_opr(&src_node);
            sr.add_depend_node(src_exit, condition);
        }
        if let Some(hs) = hold_sig    { sr.set_hold_sig_i(hs); }
        if let Some(rs) = int_rst_sig { sr.set_rst_sig_i(rs);  }
        sr.build_update_event(arena);
        let raw_state_op = sr.get_ident();
        arena.replace_back_state_reg(h, sr);

        let bound_rst = self.bind_with_rst_output_if_reset(arena, raw_state_op);
        let bound_all = self.bind_with_hold_if_hold(arena, bound_rst);
        self.state_op_i   = Some(raw_state_op);
        self.bound_exit_i = Some(bound_all);
    }

    fn get_exit_opr       (&self) -> HcpIdent { self.bound_exit_i.or(self.state_op_i).unwrap_or_default() }
    fn get_state_operating(&self) -> HcpIdent { self.state_op_i.unwrap_or_default() }
}

impl Identifiable for StateNode {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}

// ---- SynNode ----------------------------------------------------------------

pub struct SynNode {
    ident           : NcpIdent,
    clk_mode        : ClockMode,
    triggers        : NodeTrigger,
    sync_reg_i      : HcpIdent,
    syn_size        : i32,
    force_exit_node : Option<NcpIdent>,
    end_expr_i      : Option<HcpIdent>,
    bound_exit_i    : Option<HcpIdent>,
}

impl Default for SynNode {
    fn default() -> Self {
        Self {
            ident          : NcpIdent::new(NodeType::Syn, false, ""),
            clk_mode       : ClockMode::PosEdge,
            triggers       : NodeTrigger::new(),
            sync_reg_i     : HcpIdent::default(),
            syn_size       : 1,
            force_exit_node: None,
            end_expr_i     : None,
            bound_exit_i   : None,
        }
    }
}

impl SynNode {
    pub fn new(is_user_com: bool, name: &str, syn_size: i32, clk_mode: ClockMode, arena: &mut ModelArena) -> Self {
        assert!(syn_size > 0);
        let sync_reg_i = arena.make_sync_reg(&format!("{}_SY", name), syn_size);
        Self {
            ident          : NcpIdent::new(NodeType::Syn, is_user_com, name),
            clk_mode,
            triggers       : NodeTrigger::new(),
            sync_reg_i,
            syn_size,
            force_exit_node: None,
            end_expr_i     : None,
            bound_exit_i   : None,
        }
    }

    pub fn set_force_exit_event(&mut self, nd: NcpIdent) { self.force_exit_node = Some(nd); }
    pub fn get_sync_reg_i(&self) -> HcpIdent { self.sync_reg_i }
    pub fn get_syn_size  (&self) -> i32      { self.syn_size }
}

impl HasNodeTriggerSig for SynNode {
    fn get_node_triggers    (&self)     -> &NodeTrigger { &self.triggers     }
    fn get_node_triggers_mut(&mut self) -> &mut NodeTrigger { &mut self.triggers }
}

impl NcpNode for SynNode {
    fn get_ncp_ident    (&self)     -> NcpIdent     { self.ident }
    fn get_clock_mode   (&self)     -> ClockMode     { self.clk_mode }
    fn set_clock_mode   (&mut self, cm: ClockMode)   { self.clk_mode = cm; }

    fn assign(&mut self, arena: &mut ModelArena) {
        // pre-compute force-exit inverter (sense: "not force-exit")
        let force_exit_inv: Option<HcpIdent> = if let Some(fe) = self.force_exit_node {
            let fe_exit = arena.get_node_exit_opr(&fe);
            Some(arena.make_expression(
                &format!("{}_FE_INV", self.ident.get_ident_base().get_name()),
                LogicOp::BitwiseInvr, fe_exit, fe_exit,
                None, None,
            ))
        } else { None };

        let dep_list: Vec<(NcpIdent, Option<HcpIdent>)> =
            self.triggers.iter_depend_nodes().collect();
        let int_rst_sig = self.get_int_reset_node()
            .map(|h| arena.get_node_exit_opr(&h));
        let force_exit_exit = self.force_exit_node
            .map(|h| arena.get_node_exit_opr(&h));

        let h = *self.sync_reg_i.get_arena_handle();
        let mut sy = arena.take_sync_reg(h);
        for (src_node, cond) in dep_list {
            assert!(cond.is_none(), "SynNode dep nodes must not carry condition");
            let src_exit = arena.get_node_exit_opr(&src_node);
            sy.add_depend_node(src_exit, force_exit_inv);
        }
        if let Some(rs) = int_rst_sig    { sy.set_rst_sig_i(rs); }
        if let Some(fe) = force_exit_exit { sy.set_rst_sig_i(fe); }

        sy.build_support_signal(arena);
        sy.build_update_event(arena);
        let end_expr = sy.get_end_expr_i().expect("end_expr after build");
        arena.replace_back_sync_reg(h, sy);

        let bound = self.bind_with_rst_output_if_reset(arena, end_expr);
        self.end_expr_i   = Some(end_expr);
        self.bound_exit_i = Some(bound);
    }

    fn get_exit_opr  (&self) -> HcpIdent { self.bound_exit_i.or(self.end_expr_i).unwrap_or_default() }
    fn is_state_full_node(&self) -> bool             { false }
}

impl Identifiable for SynNode {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}
