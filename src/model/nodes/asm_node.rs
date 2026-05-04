use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_base::{
    add_logic_with_output, HasNodeTriggerSig, NcpNode, NodeTriggerSig,
};
use crate::model::nodes::ncp_ident::{NcpIdent, NodeType};
use crate::params::MAX_DEPEND_NODES;

/// Maximum number of `AssignMeta` entries an AsmNode can carry.
pub const MAX_ASSIGN_METAS: usize = MAX_DEPEND_NODES;

/// Assignment node — a "leaf" node owning one or more `AssignMeta`s.  In the
/// C++ flow these get attached as slaves to a `StateNode`, which calls
/// `assign_from_state_node` to gate their pre-update events with the state.
pub struct AsmNode {
    ident         : NcpIdent,
    clk_mode      : ClockMode,
    triggers      : NodeTriggerSig,
    asm_metas     : [Option<AssignMeta>; MAX_ASSIGN_METAS],
    asm_metas_len : usize,
}

impl Default for AsmNode {
    fn default() -> Self {
        Self {
            ident        : NcpIdent::new(NodeType::Asm, false, ""),
            clk_mode     : ClockMode::ClkUnused,
            triggers     : NodeTriggerSig::new(),
            asm_metas    : [None; MAX_ASSIGN_METAS],
            asm_metas_len: 0,
        }
    }
}

impl AsmNode {
    pub fn new(is_user_com: bool, name: &str, am: AssignMeta) -> Self {
        let mut node = Self {
            ident        : NcpIdent::new(NodeType::Asm, is_user_com, name),
            clk_mode     : ClockMode::ClkUnused,
            triggers     : NodeTriggerSig::new(),
            asm_metas    : [None; MAX_ASSIGN_METAS],
            asm_metas_len: 0,
        };
        node.push_assign_meta(am);
        node
    }

    pub fn new_many(is_user_com: bool, name: &str, metas: &[AssignMeta]) -> Self {
        assert!(!metas.is_empty());
        assert!(metas.len() <= MAX_ASSIGN_METAS);
        let mut node = Self {
            ident        : NcpIdent::new(NodeType::Asm, is_user_com, name),
            clk_mode     : ClockMode::ClkUnused,
            triggers     : NodeTriggerSig::new(),
            asm_metas    : [None; MAX_ASSIGN_METAS],
            asm_metas_len: 0,
        };
        for am in metas { node.push_assign_meta(*am); }
        node
    }

    pub fn push_assign_meta(&mut self, am: AssignMeta) {
        assert!(self.asm_metas_len < MAX_ASSIGN_METAS, "AsmNode capacity exceeded");
        self.asm_metas[self.asm_metas_len] = Some(am);
        self.asm_metas_len += 1;
    }

    pub fn get_assign_metas(&self) -> &[Option<AssignMeta>] {
        &self.asm_metas[..self.asm_metas_len]
    }

    pub fn add_specific_pre_condition(&mut self, cond: HcpIdent, des_idx: usize, arena: &mut ModelArena) {
        assert!(des_idx < self.asm_metas_len);
        let mut am = self.asm_metas[des_idx].expect("meta present");
        am.add_specific_pre_condition(cond, arena);
        self.asm_metas[des_idx] = Some(am);
    }

    /// Slaved variant: attach this AsmNode to a parent `StateNode`.  Walks the
    /// (single) depend node and gates each AssignMeta's pre-update event with
    /// the parent's state-operating expression and the supplied hold/reset
    /// signals.  Mirrors C++ `assignFromStateNode`.
    pub fn assign_from_state_node(
        &mut self,
        arena       : &mut ModelArena,
        hold_signal : Option<HcpIdent>,
        reset_signal: Option<HcpIdent>,
    ) {
        assert_eq!(self.triggers.depend_count(), 1, "AsmNode must have exactly one parent depend");
        assert!(self.asm_metas_len > 0);

        let (parent, condition) = self.triggers.iter_depend_nodes().next().unwrap();
        let parent_state_op = arena.get_node_state_operating(&parent);

        let mut cond_event: Option<HcpIdent> = condition;
        if let Some(hs) = hold_signal {
            let inv = arena.make_expression("asm_hold_inv", LogicOp::BitwiseInvr, hs, hs, None, None);
            cond_event = add_logic_with_output(arena, cond_event, Some(inv), LogicOp::BitwiseAnd);
        }
        if let Some(rs) = reset_signal {
            let inv = arena.make_expression("asm_rst_inv", LogicOp::BitwiseInvr, rs, rs, None, None);
            cond_event = add_logic_with_output(arena, cond_event, Some(inv), LogicOp::BitwiseAnd);
        }
        cond_event = add_logic_with_output(arena, cond_event, parent_state_op, LogicOp::BitwiseAnd);

        for slot in 0..self.asm_metas_len {
            let mut am = self.asm_metas[slot].expect("meta present");
            if let Some(c) = cond_event {
                am.add_specific_pre_condition(c, arena);
            }
            am.final_update(arena);
            self.asm_metas[slot] = Some(am);
        }
    }
}

impl HasNodeTriggerSig for AsmNode {
    fn get_node_triggers    (&self)     -> &NodeTriggerSig     { &self.triggers     }
    fn get_node_triggers_mut(&mut self) -> &mut NodeTriggerSig { &mut self.triggers }
}

impl NcpNode for AsmNode {
    fn get_ncp_ident    (&self)     -> &NcpIdent     { &self.ident }
    fn get_ncp_ident_mut(&mut self) -> &mut NcpIdent { &mut self.ident }
    fn get_clock_mode   (&self)     -> ClockMode     { self.clk_mode }
    fn set_clock_mode   (&mut self, cm: ClockMode)   { self.clk_mode = cm; }

    /// Direct `assign()` is unsupported — AsmNodes are wired through their
    /// parent StateNode via `assign_from_state_node` (or via `dry_assign` for
    /// flow-block-less binding).  Mirrors C++ `AsmNode::assign() { assert(false) }`.
    fn assign(&mut self, _arena: &mut ModelArena) {
        panic!("AsmNode::assign(): use assign_from_state_node or dry_assign");
    }

    fn dry_assign(&mut self, arena: &mut ModelArena) {
        assert!(self.asm_metas_len > 0);
        assert_eq!(self.triggers.depend_count(), 0, "dry assign must have no depend nodes");
        for slot in 0..self.asm_metas_len {
            let am = self.asm_metas[slot].expect("meta present");
            am.final_update(arena);
            self.asm_metas[slot] = Some(am);
        }
    }

    fn get_cycle_used(&self) -> i32 { 1 }
}

impl Identifiable for AsmNode {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}
