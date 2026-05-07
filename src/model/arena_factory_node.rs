use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::asm_node::AsmNode;
use crate::model::nodes::cnt_node::CounterNode;
use crate::model::nodes::logic_node::{OprNode, PseudoNode};
use crate::model::nodes::ncp_base::NcpNode;
use crate::model::nodes::ncp_ident::NcpIdent;
use crate::model::nodes::state_node::{StateNode, SynNode};
use crate::model::nodes::wait_node::{WaitCondNode, WaitCycleNode};

// make_* → is_user_com = false (internal/system)
// mk_*   → is_user_com = true  (user-defined)

impl ModelArena {
    // ---- AsmNode -----------------------------------------------------------
    pub fn make_asm_node(&mut self, name: &str, am: AssignMeta) -> NcpIdent {
        let h = self.add_asm_node(AsmNode::new(false, name, am));
        self.asm_nodes.get(h).get_ncp_ident()
    }
    pub fn mk_asm_node(&mut self, name: &str, am: AssignMeta) -> NcpIdent {
        let h = self.add_asm_node(AsmNode::new(true, name, am));
        self.asm_nodes.get(h).get_ncp_ident()
    }
    pub fn make_asm_node_many(&mut self, name: &str, ams: &[AssignMeta]) -> NcpIdent {
        let h = self.add_asm_node(AsmNode::new_many(false, name, ams));
        self.asm_nodes.get(h).get_ncp_ident()
    }

    // ---- StateNode ---------------------------------------------------------
    pub fn make_state_node(&mut self, name: &str, clk_mode: ClockMode) -> NcpIdent {
        let n = StateNode::new(false, name, clk_mode, self);
        let h = self.add_state_node(n);
        self.state_nodes.get(h).get_ncp_ident()
    }

    // ---- SynNode -----------------------------------------------------------
    pub fn make_syn_node(&mut self, name: &str, syn_size: i32, clk_mode: ClockMode) -> NcpIdent {
        let n = SynNode::new(false, name, syn_size, clk_mode, self);
        let h = self.add_syn_node(n);
        self.syn_nodes.get(h).get_ncp_ident()
    }

    // ---- WaitCondNode ------------------------------------------------------
    pub fn make_wait_cond_node(&mut self, name: &str, wait_cond: HcpIdent, clk_mode: ClockMode) -> NcpIdent {
        let n = WaitCondNode::new(false, name, wait_cond, clk_mode, self);
        let h = self.add_wait_cond_node(n);
        self.wait_cond_nodes.get(h).get_ncp_ident()
    }

    // ---- WaitCycleNode -----------------------------------------------------
    pub fn make_wait_cycle_node(&mut self, name: &str, cycle: i32, clk_mode: ClockMode) -> NcpIdent {
        let n = WaitCycleNode::new_with_cycle(false, name, cycle, clk_mode, self);
        let h = self.add_wait_cycle_node(n);
        self.wait_cycle_nodes.get(h).get_ncp_ident()
    }
    pub fn make_wait_cycle_node_with_expr(&mut self, name: &str, cnt_bit_sz: i32, end_cnt_i: HcpIdent, clk_mode: ClockMode) -> NcpIdent {
        let n = WaitCycleNode::new_with_expr(false, name, cnt_bit_sz, end_cnt_i, clk_mode, self);
        let h = self.add_wait_cycle_node(n);
        self.wait_cycle_nodes.get(h).get_ncp_ident()
    }

    // ---- CounterNode -------------------------------------------------------
    pub fn make_counter_node(&mut self, name: &str, last_loop_cnt: i32, clk_mode: ClockMode) -> NcpIdent {
        let n = CounterNode::new(false, name, last_loop_cnt, clk_mode, self);
        let h = self.add_counter_node(n);
        self.counter_nodes.get(h).get_ncp_ident()
    }

    // ---- PseudoNode --------------------------------------------------------
    pub fn make_pseudo_node(&mut self, name: &str, bit_width: i32, join_op: LogicOp) -> NcpIdent {
        let node = PseudoNode::new(false, name, bit_width, join_op, self);
        let h = self.add_pseudo_node(node);
        self.pseudo_nodes.get(h).get_ncp_ident()
    }

    // ---- OprNode -----------------------------------------------------------
    pub fn make_opr_node(&mut self, name: &str, value_i: HcpIdent) -> NcpIdent {
        let h = self.add_opr_node(OprNode::new(false, name, value_i));
        self.opr_nodes.get(h).get_ncp_ident()
    }
}
