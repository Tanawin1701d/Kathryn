use std::mem::MaybeUninit;

use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::nodes::ncp_ident::{NcpIdent, NodeType};

// ---- compile-time capacity knobs --------------------------------------------
pub const MAX_NODE_SRCS : usize = 16;
pub const MAX_CYCLE_REGS: usize = 8;

#[derive(Clone, Copy, Debug, PartialEq, Eq, Default)]
pub struct NodeTrigger {
    srci_node: Option<NcpIdent>,
    srci_cond: Option<HcpIdent>,
}

impl NodeTrigger {
    pub fn new(srci_node: Option<NcpIdent>, srci_cond: Option<HcpIdent>) -> Self {
        Self { srci_node, srci_cond }
    }

    pub fn get_trigger_signal(&self) -> Option<HcpIdent> {
        // TODO: implement
        None
    }
}

#[derive(Clone, Copy)]
pub struct NcpNodeBase {
    ncp_ident                : NcpIdent,
    node_srcs                : [NodeTrigger; MAX_NODE_SRCS],
    node_srcs_len            : usize,
    int_reset                : Option<NcpIdent>,
    hold_node                : Option<NcpIdent>,
    clk_mode                 : ClockMode,
    related_cycle_consume_reg: [MaybeUninit<HcpIdent>; MAX_CYCLE_REGS],
    reg_len                  : usize,
}

impl NcpNodeBase {
    pub fn new(node_type: NodeType, is_user_com: bool, name: &str) -> Self {
        Self {
            ncp_ident                : NcpIdent::new(node_type, is_user_com, name),
            node_srcs                : [NodeTrigger::default(); MAX_NODE_SRCS],
            node_srcs_len            : 0,
            int_reset                : None,
            hold_node                : None,
            clk_mode                 : ClockMode::ClkFree,
            related_cycle_consume_reg: [MaybeUninit::uninit(); MAX_CYCLE_REGS],
            reg_len                  : 0,
        }
    }

    pub fn get_ncp_ident    (&self)     -> &NcpIdent     { &self.ncp_ident }
    pub fn get_ncp_ident_mut(&mut self) -> &mut NcpIdent { &mut self.ncp_ident }
    pub fn get_node_type    (&self)     -> NodeType       { self.ncp_ident.get_node_type() }

    // ---- depend nodes -------------------------------------------------------

    pub fn add_depend_node(&mut self, srci_node: NcpIdent, condi: Option<HcpIdent>) {
        assert!(self.node_srcs_len < MAX_NODE_SRCS, "node_srcs capacity exceeded");
        self.node_srcs[self.node_srcs_len] = NodeTrigger::new(Some(srci_node), condi);
        self.node_srcs_len += 1;
    }

    pub fn get_depend_nodes(&self) -> &[NodeTrigger] {
        &self.node_srcs[..self.node_srcs_len]
    }

    // ---- cycle-related registers --------------------------------------------

    pub fn add_cycle_related_reg(&mut self, ctrl_reg: HcpIdent) {
        assert!(self.reg_len < MAX_CYCLE_REGS, "cycle_regs capacity exceeded");
        self.related_cycle_consume_reg[self.reg_len].write(ctrl_reg);
        self.reg_len += 1;
    }

    pub fn get_cycle_related_reg(&self) -> &[HcpIdent] {
        // SAFETY: slots [0..reg_len) are fully initialised by add_cycle_related_reg
        unsafe { std::slice::from_raw_parts(self.related_cycle_consume_reg.as_ptr() as *const HcpIdent, self.reg_len) }
    }

    // ---- clock mode ---------------------------------------------------------

    pub fn set_clock_mode(&mut self, cm: ClockMode) { self.clk_mode = cm; }
    pub fn get_clock_mode(&self)                    -> ClockMode { self.clk_mode }

    // ---- interrupt reset ----------------------------------------------------

    pub fn set_interrupt_reset(&mut self, rst: NcpIdent)   { self.int_reset = Some(rst); }
    pub fn is_there_int_reset (&self) -> bool              { self.int_reset.is_some() }
    pub fn get_interrupt_reset(&self) -> Option<&NcpIdent> { self.int_reset.as_ref() }

    // ---- hold signal --------------------------------------------------------

    pub fn set_hold(&mut self, hn: NcpIdent) {
        assert!(self.hold_node.is_none(), "hold node already set");
        self.hold_node = Some(hn);
    }
    pub fn is_there_hold(&self)  -> bool              { self.hold_node.is_some() }
    pub fn get_hold_node(&self)  -> Option<&NcpIdent> { self.hold_node.as_ref() }

    // ---- internal ident (debug label) ---------------------------------------

    pub fn set_internal_ident(&mut self, ident_val: &str) {
        self.ncp_ident.get_ident_base_mut().set_name(ident_val);
    }

    pub fn get_md_ident_val(&self) -> String {
        format!("{} @ {:p}", self.ncp_ident.get_node_type(), self)
    }

    // ---- static logic helpers -----------------------------------------------

    /// Combine `des_logic` with `opr1` using `op` (BitwiseAnd / BitwiseOr).
    /// If `des_logic` is `None`, it is set to `opr1`; otherwise a new combined
    /// ident handle must be created by the caller via the arena.
    pub fn add_logic(des_logic: &mut Option<HcpIdent>, opr1: HcpIdent, op: LogicOp) {
        assert!(op == LogicOp::BitwiseAnd || op == LogicOp::BitwiseOr);
        if des_logic.is_none() {
            *des_logic = Some(opr1);
        } else {
            // Arena-backed expression creation is handled at call sites.
            todo!("combine expressions through arena")
        }
    }

    /// Return a combined ident handle for `opr1 op opr2`.
    /// If both are `None` returns `None`; if one side is `None` returns the other.
    pub fn add_logic_with_output(
        opr1: Option<HcpIdent>,
        opr2: Option<HcpIdent>,
        op  : LogicOp,
    ) -> Option<HcpIdent> {
        assert!(op == LogicOp::BitwiseAnd || op == LogicOp::BitwiseOr);
        match (opr1, opr2) {
            (None,    None   ) => None,
            (Some(a), None   ) => Some(a),
            (None,    Some(b)) => Some(b),
            (Some(_), Some(_)) => todo!("combine expressions through arena"),
        }
    }
}

/// Trait for all concrete node types.  Mirrors the pure-virtual / virtual
pub trait NcpNode {
    fn get_ncp_node_base    (&self)     -> &NcpNodeBase;
    fn get_ncp_node_base_mut(&mut self) -> &mut NcpNodeBase;

    // ---- pure virtual -------------------------------------------------------
    fn assign        (&mut self);
    fn get_cycle_used(&self) -> i32;

    // ---- virtual with defaults ----------------------------------------------
    fn get_exit_opr          (&self)     -> Option<HcpIdent> { None }
    fn get_state_operating   (&self)     -> Option<HcpIdent> { None }
    fn dry_assign            (&mut self) { panic!("dry_assign: not implemented") }
    fn is_state_full_node    (&self)     -> bool { true }

    // ---- forwarded helpers --------------------------------------------------
    fn get_node_type(&self) -> NodeType {
        self.get_ncp_node_base().get_node_type()
    }

    fn get_md_ident_val(&self) -> String {
        self.get_ncp_node_base().get_md_ident_val()
    }

    fn bind_with_rst_output_if_reset(&self, raw_exit: HcpIdent) -> HcpIdent {
        if self.get_ncp_node_base().is_there_int_reset() {
            todo!("bind with interrupt-reset expression via arena")
        }
        raw_exit
    }

    fn bind_with_hold_if_hold(&self, raw_exit: HcpIdent) -> HcpIdent {
        if self.get_ncp_node_base().is_there_hold() {
            todo!("bind with hold expression via arena")
        }
        raw_exit
    }
}
