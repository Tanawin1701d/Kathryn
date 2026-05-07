use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
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
        let sig = self.triggers.to_trigger_sig(arena);
        let mut sr = arena.take_state_reg(self.state_reg_i);
        *sr.get_triggers_mut() = sig;
        sr.build_update_event(arena);
        let raw_state_op = sr.get_ident();
        arena.replace_back_state_reg(sr);

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

