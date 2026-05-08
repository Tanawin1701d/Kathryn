use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_base::{
    add_logic_with_output, HasNodeTriggerSig, NcpNode, NodeTrigger,
};
use crate::model::nodes::ncp_ident::{NcpIdent, NodeType};

/// Assignment node — a "leaf" node owning one or more `AssignMeta`s.  In the
/// C++ flow these get attached as slaves to a `StateNode`, which calls
/// `assign_from_state_node` to gate their pre-update events with the state.
pub struct AsmNode {
    ident    : NcpIdent,
    clk_mode : ClockMode, /// TODO now clock mode is reduntdant
    triggers : NodeTrigger,
    asm_metas: Vec<AssignMeta>,
}

impl Default for AsmNode {
    fn default() -> Self {
        Self {
            ident    : NcpIdent::new(NodeType::Asm, false, ""),
            clk_mode : ClockMode::ClkUnused,
            triggers : NodeTrigger::new(),
            asm_metas: Vec::new(),
        }
    }
}

impl AsmNode {
    pub fn new(is_user_com: bool, name: &str, am: AssignMeta) -> Self {
        Self {
            ident    : NcpIdent::new(NodeType::Asm, is_user_com, name),
            clk_mode : ClockMode::ClkUnused,
            triggers : NodeTrigger::new(),
            asm_metas: vec![am],
        }
    }

    pub fn new_many(is_user_com: bool, name: &str, metas: &[AssignMeta]) -> Self {
        assert!(!metas.is_empty());
        Self {
            ident    : NcpIdent::new(NodeType::Asm, is_user_com, name),
            clk_mode : ClockMode::ClkUnused,
            triggers : NodeTrigger::new(),
            asm_metas: metas.to_vec(),
        }
    }

    pub fn push_assign_meta(&mut self, am: AssignMeta) {
        self.asm_metas.push(am);
    }

    pub fn get_assign_metas(&self) -> &[AssignMeta] {
        &self.asm_metas
    }

    pub fn add_specific_pre_condition(&mut self, cond: HcpIdent, des_idx: usize, arena: &mut ModelArena) {
        assert!(des_idx < self.asm_metas.len());
        self.asm_metas[des_idx].add_specific_pre_condition(cond, arena);
    }

    /// Slaved variant: attach this AsmNode to a parent `StateNode`.  Walks the
    /// (single) depend node and gates each AssignMeta's pre-update event with
    /// the parent's state-operating expression and the supplied hold/reset
    /// signals.  Mirrors C++ `assignFromStateNode`.
    pub fn assign_from_state_node(&mut self, arena: &mut ModelArena) {
        assert_eq!(self.triggers.depend_count(), 1, "AsmNode must have exactly one parent depend");
        assert!(!self.asm_metas.is_empty());

        let (parent, condition) = self.triggers.iter_depend_nodes().next().unwrap();
        let hold_signal  = self.triggers.hold_node_i     .map(|n| arena.get_node_exit_opr(&n));
        let reset_signal = self.triggers.int_reset_node_i.map(|n| arena.get_node_exit_opr(&n));
        let parent_state_op = Some(arena.get_node_state_operating(&parent));

        let mut cond_event: Option<HcpIdent> = condition;
        if let Some(hs) = hold_signal {
            let inv = arena.make_expression("asm_hold_inv", LogicOp::BitwiseInvr, hs, HcpIdent::default(), None, None);
            cond_event = add_logic_with_output(arena, cond_event, Some(inv), LogicOp::BitwiseAnd);
        }
        if let Some(rs) = reset_signal {
            let inv = arena.make_expression("asm_rst_inv", LogicOp::BitwiseInvr, rs, HcpIdent::default(), None, None);
            cond_event = add_logic_with_output(arena, cond_event, Some(inv), LogicOp::BitwiseAnd);
        }
        cond_event = add_logic_with_output(arena, cond_event, parent_state_op, LogicOp::BitwiseAnd);

        for am in &mut self.asm_metas {
            if let Some(c) = cond_event {
                am.add_specific_pre_condition(c, arena);
            }
            am.final_update(arena);
        }
    }
}

impl HasNodeTriggerSig for AsmNode {
    fn get_node_triggers    (&self)     -> &NodeTrigger { &self.triggers     }
    fn get_node_triggers_mut(&mut self) -> &mut NodeTrigger { &mut self.triggers }
}

impl NcpNode for AsmNode {
    fn get_ncp_ident    (&self)     -> NcpIdent     { self.ident }
    fn get_clock_mode   (&self)     -> ClockMode     { self.clk_mode }
    fn set_clock_mode   (&mut self, cm: ClockMode)   { self.clk_mode = cm; }

    /// Direct `assign()` is unsupported — AsmNodes are wired through their
    /// parent StateNode via `assign_from_state_node` (or via `dry_assign` for
    /// flow-block-less binding).  Mirrors C++ `AsmNode::assign() { assert(false) }`.
    fn assign(&mut self, _arena: &mut ModelArena) {
        panic!("AsmNode::assign(): use assign_from_state_node or dry_assign");
    }

    fn dry_assign(&mut self, arena: &mut ModelArena) {
        assert!(!self.asm_metas.is_empty());
        assert_eq!(self.triggers.depend_count(), 0, "dry assign must have no depend nodes");
        for am in self.asm_metas.iter().copied() {
            am.final_update(arena);
        }
    }
}

impl Identifiable for AsmNode {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}
