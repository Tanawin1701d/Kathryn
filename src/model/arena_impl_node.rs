use crate::common::arena_base::ArenaHandle;
use crate::model::common::identifier::Identifiable;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::asm_node::AsmNode;
use crate::model::nodes::cnt_node::CounterNode;
use crate::model::nodes::logic_node::{OprNode, PseudoNode};
use crate::model::nodes::ncp_base::NcpNode;
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
    pub fn add_asm_node       (&mut self, n: AsmNode)        -> ArenaHandle { self.asm_nodes       .insert(n) }
    pub fn add_start_node     (&mut self, n: StartNode)      -> ArenaHandle { self.start_nodes     .insert(n) }
    pub fn add_state_node     (&mut self, n: StateNode)      -> ArenaHandle { self.state_nodes     .insert(n) }
    pub fn add_syn_node       (&mut self, n: SynNode)        -> ArenaHandle { self.syn_nodes       .insert(n) }
    pub fn add_wait_cond_node (&mut self, n: WaitCondNode)   -> ArenaHandle { self.wait_cond_nodes .insert(n) }
    pub fn add_wait_cycle_node(&mut self, n: WaitCycleNode)  -> ArenaHandle { self.wait_cycle_nodes.insert(n) }
    pub fn add_counter_node   (&mut self, n: CounterNode)    -> ArenaHandle { self.counter_nodes   .insert(n) }
    pub fn add_pseudo_node    (&mut self, n: PseudoNode)     -> ArenaHandle { self.pseudo_nodes    .insert(n) }
    pub fn add_opr_node       (&mut self, n: OprNode)        -> ArenaHandle { self.opr_nodes       .insert(n) }

    // ----- take / replace_back (use these instead of typed get/get_mut) ----
    pub fn take_asm_node       (&mut self, h: ArenaHandle) -> AsmNode        { self.asm_nodes       .take(h) }
    pub fn take_start_node     (&mut self, h: ArenaHandle) -> StartNode      { self.start_nodes     .take(h) }
    pub fn take_state_node     (&mut self, h: ArenaHandle) -> StateNode      { self.state_nodes     .take(h) }
    pub fn take_syn_node       (&mut self, h: ArenaHandle) -> SynNode        { self.syn_nodes       .take(h) }
    pub fn take_wait_cond_node (&mut self, h: ArenaHandle) -> WaitCondNode   { self.wait_cond_nodes .take(h) }
    pub fn take_wait_cycle_node(&mut self, h: ArenaHandle) -> WaitCycleNode  { self.wait_cycle_nodes.take(h) }
    pub fn take_counter_node   (&mut self, h: ArenaHandle) -> CounterNode    { self.counter_nodes   .take(h) }
    pub fn take_pseudo_node    (&mut self, h: ArenaHandle) -> PseudoNode     { self.pseudo_nodes    .take(h) }
    pub fn take_opr_node       (&mut self, h: ArenaHandle) -> OprNode        { self.opr_nodes       .take(h) }

    pub fn replace_back_asm_node       (&mut self, h: ArenaHandle, v: AsmNode      ) { self.asm_nodes       .replace_back(h, v) }
    pub fn replace_back_start_node     (&mut self, h: ArenaHandle, v: StartNode    ) { self.start_nodes     .replace_back(h, v) }
    pub fn replace_back_state_node     (&mut self, h: ArenaHandle, v: StateNode    ) { self.state_nodes     .replace_back(h, v) }
    pub fn replace_back_syn_node       (&mut self, h: ArenaHandle, v: SynNode      ) { self.syn_nodes       .replace_back(h, v) }
    pub fn replace_back_wait_cond_node (&mut self, h: ArenaHandle, v: WaitCondNode ) { self.wait_cond_nodes .replace_back(h, v) }
    pub fn replace_back_wait_cycle_node(&mut self, h: ArenaHandle, v: WaitCycleNode) { self.wait_cycle_nodes.replace_back(h, v) }
    pub fn replace_back_counter_node   (&mut self, h: ArenaHandle, v: CounterNode  ) { self.counter_nodes   .replace_back(h, v) }
    pub fn replace_back_pseudo_node    (&mut self, h: ArenaHandle, v: PseudoNode   ) { self.pseudo_nodes    .replace_back(h, v) }
    pub fn replace_back_opr_node       (&mut self, h: ArenaHandle, v: OprNode      ) { self.opr_nodes       .replace_back(h, v) }

    // ----- trait-object dispatch (cannot be expressed via take/replace) ----
    pub fn get_ncp_node    (&self,     ident: &NcpIdent) -> &    dyn NcpNode { dispatch_ncp!(self, ident, get    ) }
    pub fn get_ncp_node_mut(&mut self, ident: &NcpIdent) -> &mut dyn NcpNode { dispatch_ncp!(self, ident, get_mut) }

    pub fn get_node_exit_opr       (&self, ident: &NcpIdent) -> HcpIdent { self.get_ncp_node(ident).get_exit_opr() }
    pub fn get_node_state_operating(&self, ident: &NcpIdent) -> HcpIdent { self.get_ncp_node(ident).get_state_operating() }
    pub fn get_node_cycle_used     (&self, ident: &NcpIdent) -> i32      { self.get_ncp_node(ident).get_cycle_used() }

    pub fn add_depend_node_to_ncp(&mut self, ident: NcpIdent, src: NcpIdent, cond: Option<HcpIdent>) {
        self.get_ncp_node_mut(&ident).add_depend_node(src, cond);
    }

    pub fn set_ncp_int_reset_node(&mut self, ident: NcpIdent, rst: NcpIdent) {
        self.get_ncp_node_mut(&ident).set_int_reset_node(rst);
    }

    pub fn set_ncp_hold_node(&mut self, ident: NcpIdent, hold: NcpIdent) {
        self.get_ncp_node_mut(&ident).set_hold_node(hold);
    }

    pub fn add_slave_asm_to_state_node(&mut self, state: NcpIdent, asm: NcpIdent, cond: Option<HcpIdent>) {
        assert_eq!(state.get_node_type(), NodeType::State);
        assert_eq!(asm.get_node_type(), NodeType::Asm);
        let handle = *state.get_arena_handle();
        self.state_nodes.get_mut(handle).add_slave_asm_node(asm, cond);
        self.add_depend_node_to_ncp(asm, state, cond);
    }

    pub fn assign_asm_from_state_node(&mut self, ident: NcpIdent) {
        assert_eq!(ident.get_node_type(), NodeType::Asm);
        let handle = *ident.get_arena_handle();
        let mut node = self.take_asm_node(handle);
        node.assign_from_state_node(self);
        self.replace_back_asm_node(handle, node);
    }

    pub fn assign_ncp_node(&mut self, ident: NcpIdent) {
        let handle = *ident.get_arena_handle();
        match ident.get_node_type() {
            NodeType::Asm => {
                let mut node = self.take_asm_node(handle);
                node.assign(self);
                self.replace_back_asm_node(handle, node);
            }
            NodeType::Start => {
                let mut node = self.take_start_node(handle);
                node.assign(self);
                self.replace_back_start_node(handle, node);
            }
            NodeType::State => {
                let mut node = self.take_state_node(handle);
                node.assign(self);
                self.replace_back_state_node(handle, node);
            }
            NodeType::Syn => {
                let mut node = self.take_syn_node(handle);
                node.assign(self);
                self.replace_back_syn_node(handle, node);
            }
            NodeType::WaitCond => {
                let mut node = self.take_wait_cond_node(handle);
                node.assign(self);
                self.replace_back_wait_cond_node(handle, node);
            }
            NodeType::WaitCycle => {
                let mut node = self.take_wait_cycle_node(handle);
                node.assign(self);
                self.replace_back_wait_cycle_node(handle, node);
            }
            NodeType::Counter => {
                let mut node = self.take_counter_node(handle);
                node.assign(self);
                self.replace_back_counter_node(handle, node);
            }
            NodeType::Pseudo => {
                let mut node = self.take_pseudo_node(handle);
                node.assign(self);
                self.replace_back_pseudo_node(handle, node);
            }
            NodeType::Opr => panic!("OprNode does not support assign"),
        }
    }
}
