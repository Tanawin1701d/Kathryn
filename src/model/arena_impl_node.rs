use crate::model::common::identifier::Identifiable;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::asm_node::AsmNode;
use crate::model::nodes::cnt_node::CounterNode;
use crate::model::nodes::logic_node::{OprNode, PseudoNode};
use crate::model::nodes::ncp_base::{HasNodeTriggerSig, NcpNode};
use crate::model::nodes::node_trigger::NodeTrigger;
use crate::model::nodes::ncp_ident::{NcpIdent, NodeType};
use crate::model::nodes::start_node::StartNode;
use crate::model::nodes::state_node::StateNode;
use crate::model::nodes::syn_node::SynNode;
use crate::model::nodes::wait_node::{WaitCondNode, WaitCycleNode};

macro_rules! dispatch_ncp {
    ($self:expr, $ident:expr, $method:ident) => {{
        let handle = *$ident.get_arena_handle();
        match $ident.get_node_type() {
            NodeType::Asm       => $self.asm_nodes       .$method(handle) as _,
            NodeType::Start     => $self.start_nodes     .$method(handle) as _,
            NodeType::State     => $self.state_nodes     .$method(handle) as _,
            NodeType::Syn       => $self.syn_nodes       .$method(handle) as _,
            NodeType::WaitCond  => $self.wait_cond_nodes .$method(handle) as _,
            NodeType::WaitCycle => $self.wait_cycle_nodes.$method(handle) as _,
            NodeType::Counter   => $self.counter_nodes   .$method(handle) as _,
            NodeType::Pseudo    => $self.pseudo_nodes    .$method(handle) as _,
            NodeType::Opr       => $self.opr_nodes       .$method(handle) as _,
        }
    }};
}

impl ModelArena {
    // ----- inserts ---------------------------------------------------------
    pub fn add_asm_node       (&mut self, n: AsmNode)       -> NcpIdent { let h = self.asm_nodes       .insert(n); self.asm_nodes       .get(h).get_ncp_ident() }
    pub fn add_start_node     (&mut self, n: StartNode)     -> NcpIdent { let h = self.start_nodes     .insert(n); self.start_nodes     .get(h).get_ncp_ident() }
    pub fn add_state_node     (&mut self, n: StateNode)     -> NcpIdent { let h = self.state_nodes     .insert(n); self.state_nodes     .get(h).get_ncp_ident() }
    pub fn add_syn_node       (&mut self, n: SynNode)       -> NcpIdent { let h = self.syn_nodes       .insert(n); self.syn_nodes       .get(h).get_ncp_ident() }
    pub fn add_wait_cond_node (&mut self, n: WaitCondNode)  -> NcpIdent { let h = self.wait_cond_nodes .insert(n); self.wait_cond_nodes .get(h).get_ncp_ident() }
    pub fn add_wait_cycle_node(&mut self, n: WaitCycleNode) -> NcpIdent { let h = self.wait_cycle_nodes.insert(n); self.wait_cycle_nodes.get(h).get_ncp_ident() }
    pub fn add_counter_node   (&mut self, n: CounterNode)   -> NcpIdent { let h = self.counter_nodes   .insert(n); self.counter_nodes   .get(h).get_ncp_ident() }
    pub fn add_pseudo_node    (&mut self, n: PseudoNode)    -> NcpIdent { let h = self.pseudo_nodes    .insert(n); self.pseudo_nodes    .get(h).get_ncp_ident() }
    pub fn add_opr_node       (&mut self, n: OprNode)       -> NcpIdent { let h = self.opr_nodes       .insert(n); self.opr_nodes       .get(h).get_ncp_ident() }

    // ----- take / replace_back (use these instead of typed get/get_mut) ----
    pub fn take_asm_node       (&mut self, i: NcpIdent) -> AsmNode        { self.asm_nodes       .take(*i.get_arena_handle()) }
    pub fn take_start_node     (&mut self, i: NcpIdent) -> StartNode      { self.start_nodes     .take(*i.get_arena_handle()) }
    pub fn take_state_node     (&mut self, i: NcpIdent) -> StateNode      { self.state_nodes     .take(*i.get_arena_handle()) }
    pub fn take_syn_node       (&mut self, i: NcpIdent) -> SynNode        { self.syn_nodes       .take(*i.get_arena_handle()) }
    pub fn take_wait_cond_node (&mut self, i: NcpIdent) -> WaitCondNode   { self.wait_cond_nodes .take(*i.get_arena_handle()) }
    pub fn take_wait_cycle_node(&mut self, i: NcpIdent) -> WaitCycleNode  { self.wait_cycle_nodes.take(*i.get_arena_handle()) }
    pub fn take_counter_node   (&mut self, i: NcpIdent) -> CounterNode    { self.counter_nodes   .take(*i.get_arena_handle()) }
    pub fn take_pseudo_node    (&mut self, i: NcpIdent) -> PseudoNode     { self.pseudo_nodes    .take(*i.get_arena_handle()) }
    pub fn take_opr_node       (&mut self, i: NcpIdent) -> OprNode        { self.opr_nodes       .take(*i.get_arena_handle()) }

    pub fn replace_back_asm_node       (&mut self, v: AsmNode      ) { let h = *v.get_arena_handle(); self.asm_nodes       .replace_back(h, v) }
    pub fn replace_back_start_node     (&mut self, v: StartNode    ) { let h = *v.get_arena_handle(); self.start_nodes     .replace_back(h, v) }
    pub fn replace_back_state_node     (&mut self, v: StateNode    ) { let h = *v.get_arena_handle(); self.state_nodes     .replace_back(h, v) }
    pub fn replace_back_syn_node       (&mut self, v: SynNode      ) { let h = *v.get_arena_handle(); self.syn_nodes       .replace_back(h, v) }
    pub fn replace_back_wait_cond_node (&mut self, v: WaitCondNode ) { let h = *v.get_arena_handle(); self.wait_cond_nodes .replace_back(h, v) }
    pub fn replace_back_wait_cycle_node(&mut self, v: WaitCycleNode) { let h = *v.get_arena_handle(); self.wait_cycle_nodes.replace_back(h, v) }
    pub fn replace_back_counter_node   (&mut self, v: CounterNode  ) { let h = *v.get_arena_handle(); self.counter_nodes   .replace_back(h, v) }
    pub fn replace_back_pseudo_node    (&mut self, v: PseudoNode   ) { let h = *v.get_arena_handle(); self.pseudo_nodes    .replace_back(h, v) }
    pub fn replace_back_opr_node       (&mut self, v: OprNode      ) { let h = *v.get_arena_handle(); self.opr_nodes       .replace_back(h, v) }

    // ----- polymorphic take / replace_back -----------------------------------
    pub fn take_ncp_node(&mut self, ident: NcpIdent) -> Box<dyn NcpNode> {
        match ident.get_node_type() {
            NodeType::Asm       => Box::new(self.take_asm_node       (ident)),
            NodeType::Start     => Box::new(self.take_start_node     (ident)),
            NodeType::State     => Box::new(self.take_state_node     (ident)),
            NodeType::Syn       => Box::new(self.take_syn_node       (ident)),
            NodeType::WaitCond  => Box::new(self.take_wait_cond_node (ident)),
            NodeType::WaitCycle => Box::new(self.take_wait_cycle_node(ident)),
            NodeType::Counter   => Box::new(self.take_counter_node   (ident)),
            NodeType::Pseudo    => Box::new(self.take_pseudo_node    (ident)),
            NodeType::Opr       => Box::new(self.take_opr_node       (ident)),
        }
    }

    pub fn replace_back_ncp_node(&mut self, node: Box<dyn NcpNode>) {
        node.replace_back_into_arena(self);
    }

    // ----- trait-object dispatch (cannot be expressed via take/replace) ----
    fn get_ncp_node    (&self,     ident: &NcpIdent) -> &    dyn NcpNode { dispatch_ncp!(self, ident, get    ) }
    fn get_ncp_node_mut(&mut self, ident: &NcpIdent) -> &mut dyn NcpNode { dispatch_ncp!(self, ident, get_mut) }

    pub fn get_node_exit_opr       (&self, ident: &NcpIdent) -> HcpIdent    { self.get_ncp_node(ident).get_exit_opr() }
    pub fn get_node_state_operating(&self, ident: &NcpIdent) -> HcpIdent    { self.get_ncp_node(ident).get_state_operating() }
    pub fn get_node_cycle_used     (&self, ident: &NcpIdent) -> i32         { self.get_ncp_node(ident).get_cycle_used() }
    pub fn get_ncp_trigger_clone   (&self, ident: &NcpIdent) -> NodeTrigger { self.get_ncp_node(ident).get_node_triggers().clone() }

    pub fn add_depend_node_to_ncp(&mut self, des_node_i: NcpIdent, src_node_i: NcpIdent, cond: Option<HcpIdent>) {
        self.get_ncp_node_mut(&des_node_i).add_depend_node(src_node_i, cond);
    }

    pub fn set_ncp_int_reset_node(&mut self, ident: NcpIdent, rst: NcpIdent) {
        self.get_ncp_node_mut(&ident).set_int_reset_node(rst);
    }

    pub fn set_ncp_hold_node(&mut self, ident: NcpIdent, hold: NcpIdent) {
        self.get_ncp_node_mut(&ident).set_hold_node(hold);
    }

    pub fn init_node_trigger(&mut self, ident: NcpIdent, trigger: &NodeTrigger, with_int_start: bool) {
        self.get_ncp_node_mut(&ident).fill_ext_node(trigger, with_int_start);
    }

    /// Propagate the AsmNode's wired clock source onto each of its AssignMeta UEs.
    /// No-op if the AsmNode's trigger has no clk_node_i set.
    pub fn init_asm_node_clk_src(&mut self, ident: NcpIdent) {
        assert_eq!(ident.get_node_type(), NodeType::Asm);
        let mut asm = self.take_asm_node(ident);
        asm.set_clk_src(self);
        self.replace_back_asm_node(asm);
    }

    // assign for asm node

    pub fn assign_asm_from_state_node(&mut self, ident: NcpIdent) {
        assert_eq!(ident.get_node_type(), NodeType::Asm);
        let mut node = self.take_asm_node(ident);
        node.assign_from_state_node(self);
        self.replace_back_asm_node(node);
    }

    pub fn dry_assign_asm_node(&mut self, ident: NcpIdent) {
        assert_eq!(ident.get_node_type(), NodeType::Asm);
        let mut node = self.take_asm_node(ident);
        node.dry_assign(self);
        self.replace_back_asm_node(node);
    }

    // assign the node

    pub fn assign_ncp_node(&mut self, ident: NcpIdent) {
        let mut node = self.take_ncp_node(ident);
        node.assign_final(self);
        self.replace_back_ncp_node(node);
    }
}
