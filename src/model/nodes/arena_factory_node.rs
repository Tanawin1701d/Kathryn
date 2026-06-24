use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::hw_component::common::slice::Slice;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::asm_node::AsmNode;
use crate::model::nodes::cnt_node::CounterNode;
use crate::model::nodes::logic_node::{OprNode, PseudoNode};
use crate::model::nodes::ncp_ident::NcpIdent;
use crate::model::nodes::start_node::StartNode;
use crate::model::nodes::state_node::StateNode;
use crate::model::nodes::syn_node::SynNode;
use crate::model::nodes::wait_node::{WaitCondNode, WaitCycleNode};

// make_* → is_user_com = false (internal/system)
// mk_*   → is_user_com = true  (user-defined)

impl ModelArena {
    // ---- StartNode ---------------------------------------------------------
    pub fn make_start_node(&mut self, name: &str, rst_sig_i: HcpIdent) -> NcpIdent {
        let n = StartNode::new(name, rst_sig_i, self);
        self.add_start_node(n)
    }

    // ---- AsmNode -----------------------------------------------------------
    pub fn make_asm_node(&mut self, is_user_com: bool, name: &str, am: AssignMeta) -> NcpIdent {
        self.add_asm_node(AsmNode::new(is_user_com, name, am))
    }
    pub fn make_asm_node_many(&mut self, name: &str, ams: &[AssignMeta]) -> NcpIdent {
        self.add_asm_node(AsmNode::new_many(false, name, ams))
    }

    // ---- StateNode ---------------------------------------------------------
    pub fn make_state_node(&mut self, name: &str) -> NcpIdent {
        let n = StateNode::new(false, name, self);
        self.add_state_node(n)
    }

    // ---- SynNode -----------------------------------------------------------
    pub fn make_syn_node(&mut self, name: &str, syn_size: i32) -> NcpIdent {
        let n = SynNode::new(false, name, syn_size, self);
        self.add_syn_node(n)
    }

    // ---- WaitCondNode ------------------------------------------------------
    pub fn make_wait_cond_node(&mut self, name: &str, wait_cond: HcpIdent, wait_con_sl: Slice) -> NcpIdent {
        let n = WaitCondNode::new(false, name, wait_cond, wait_con_sl, self);
        self.add_wait_cond_node(n)
    }

    // ---- WaitCycleNode -----------------------------------------------------
    pub fn make_wait_cycle_node(&mut self, name: &str, cycle: i32) -> NcpIdent {
        let n = WaitCycleNode::new_with_cycle(false, name, cycle, self);
        self.add_wait_cycle_node(n)
    }
    pub fn make_wait_cycle_node_with_expr(&mut self, name: &str, cnt_bit_sz: i32, end_cnt_i: HcpIdent) -> NcpIdent {
        let n = WaitCycleNode::new_with_expr(false, name, cnt_bit_sz, end_cnt_i, self);
        self.add_wait_cycle_node(n)
    }

    // ---- CounterNode -------------------------------------------------------
    pub fn make_counter_node(&mut self, name: &str, last_loop_cnt: i32) -> NcpIdent {
        let n = CounterNode::new(false, name, last_loop_cnt, self);
        self.add_counter_node(n)
    }

    // ---- PseudoNode --------------------------------------------------------
    pub fn make_pseudo_node(&mut self, name: &str, bit_width: i32, join_op: LogicOp) -> NcpIdent {
        let node = PseudoNode::new(false, name, bit_width, join_op, self);
        self.add_pseudo_node(node)
    }

    // ---- OprNode -----------------------------------------------------------
    pub fn make_opr_node(&mut self, name: &str, value_i: HcpIdent) -> NcpIdent {
        self.add_opr_node(OprNode::new(false, name, value_i))
    }
}
