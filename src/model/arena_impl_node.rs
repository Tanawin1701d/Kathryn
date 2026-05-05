use crate::common::arena_base::ArenaHandle;
use crate::model::common::identifier::Identifiable;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::asm_node::AsmNode;
use crate::model::nodes::cnt_node::CounterNode;
use crate::model::nodes::logic_node::{DummyNode, OprNode, PseudoNode};
use crate::model::nodes::ncp_base::NcpNode;
use crate::model::nodes::ncp_ident::{NcpIdent, NodeType};
use crate::model::nodes::state_node::{StateNode, SynNode};
use crate::model::nodes::wait_node::{WaitCondNode, WaitCycleNode};

macro_rules! dispatch_ncp {
    ($self:expr, $ident:expr, $method:ident) => {{
        let handle = *$ident.get_arena_handle();
        match $ident.get_node_type() {
            NodeType::Asm       => $self.asm_nodes       .$method(handle) as _,
            NodeType::State     => $self.state_nodes     .$method(handle) as _,
            NodeType::Syn       => $self.syn_nodes       .$method(handle) as _,
            NodeType::WaitCond  => $self.wait_cond_nodes .$method(handle) as _,
            NodeType::WaitCycle => $self.wait_cycle_nodes.$method(handle) as _,
            NodeType::Counter   => $self.counter_nodes   .$method(handle) as _,
            NodeType::Pseudo    => $self.pseudo_nodes    .$method(handle) as _,
            NodeType::Dummy     => $self.dummy_nodes     .$method(handle) as _,
            NodeType::Opr       => $self.opr_nodes       .$method(handle) as _,
        }
    }};
}

impl ModelArena {
    // ----- inserts ---------------------------------------------------------
    pub fn add_asm_node       (&mut self, n: AsmNode)        -> ArenaHandle { self.asm_nodes       .insert(n) }
    pub fn add_state_node     (&mut self, n: StateNode)      -> ArenaHandle { self.state_nodes     .insert(n) }
    pub fn add_syn_node       (&mut self, n: SynNode)        -> ArenaHandle { self.syn_nodes       .insert(n) }
    pub fn add_wait_cond_node (&mut self, n: WaitCondNode)   -> ArenaHandle { self.wait_cond_nodes .insert(n) }
    pub fn add_wait_cycle_node(&mut self, n: WaitCycleNode)  -> ArenaHandle { self.wait_cycle_nodes.insert(n) }
    pub fn add_counter_node   (&mut self, n: CounterNode)    -> ArenaHandle { self.counter_nodes   .insert(n) }
    pub fn add_pseudo_node    (&mut self, n: PseudoNode)     -> ArenaHandle { self.pseudo_nodes    .insert(n) }
    pub fn add_dummy_node     (&mut self, n: DummyNode)      -> ArenaHandle { self.dummy_nodes     .insert(n) }
    pub fn add_opr_node       (&mut self, n: OprNode)        -> ArenaHandle { self.opr_nodes       .insert(n) }

    // ----- getters ---------------------------------------------------------
    pub fn get_asm_node       (&self, h: ArenaHandle) -> &AsmNode        { self.asm_nodes       .get(h) }
    pub fn get_state_node     (&self, h: ArenaHandle) -> &StateNode      { self.state_nodes     .get(h) }
    pub fn get_syn_node       (&self, h: ArenaHandle) -> &SynNode        { self.syn_nodes       .get(h) }
    pub fn get_wait_cond_node (&self, h: ArenaHandle) -> &WaitCondNode   { self.wait_cond_nodes .get(h) }
    pub fn get_wait_cycle_node(&self, h: ArenaHandle) -> &WaitCycleNode  { self.wait_cycle_nodes.get(h) }
    pub fn get_counter_node   (&self, h: ArenaHandle) -> &CounterNode    { self.counter_nodes   .get(h) }
    pub fn get_pseudo_node    (&self, h: ArenaHandle) -> &PseudoNode     { self.pseudo_nodes    .get(h) }
    pub fn get_dummy_node     (&self, h: ArenaHandle) -> &DummyNode      { self.dummy_nodes     .get(h) }
    pub fn get_opr_node       (&self, h: ArenaHandle) -> &OprNode        { self.opr_nodes       .get(h) }

    pub fn get_asm_node_mut       (&mut self, h: ArenaHandle) -> &mut AsmNode        { self.asm_nodes       .get_mut(h) }
    pub fn get_state_node_mut     (&mut self, h: ArenaHandle) -> &mut StateNode      { self.state_nodes     .get_mut(h) }
    pub fn get_syn_node_mut       (&mut self, h: ArenaHandle) -> &mut SynNode        { self.syn_nodes       .get_mut(h) }
    pub fn get_wait_cond_node_mut (&mut self, h: ArenaHandle) -> &mut WaitCondNode   { self.wait_cond_nodes .get_mut(h) }
    pub fn get_wait_cycle_node_mut(&mut self, h: ArenaHandle) -> &mut WaitCycleNode  { self.wait_cycle_nodes.get_mut(h) }
    pub fn get_counter_node_mut   (&mut self, h: ArenaHandle) -> &mut CounterNode    { self.counter_nodes   .get_mut(h) }
    pub fn get_pseudo_node_mut    (&mut self, h: ArenaHandle) -> &mut PseudoNode     { self.pseudo_nodes    .get_mut(h) }
    pub fn get_dummy_node_mut     (&mut self, h: ArenaHandle) -> &mut DummyNode      { self.dummy_nodes     .get_mut(h) }
    pub fn get_opr_node_mut       (&mut self, h: ArenaHandle) -> &mut OprNode        { self.opr_nodes       .get_mut(h) }

    // ----- take / replace_back --------------------------------------------
    pub fn take_asm_node       (&mut self, h: ArenaHandle) -> AsmNode        { self.asm_nodes       .take(h) }
    pub fn take_state_node     (&mut self, h: ArenaHandle) -> StateNode      { self.state_nodes     .take(h) }
    pub fn take_syn_node       (&mut self, h: ArenaHandle) -> SynNode        { self.syn_nodes       .take(h) }
    pub fn take_wait_cond_node (&mut self, h: ArenaHandle) -> WaitCondNode   { self.wait_cond_nodes .take(h) }
    pub fn take_wait_cycle_node(&mut self, h: ArenaHandle) -> WaitCycleNode  { self.wait_cycle_nodes.take(h) }
    pub fn take_counter_node   (&mut self, h: ArenaHandle) -> CounterNode    { self.counter_nodes   .take(h) }
    pub fn take_pseudo_node    (&mut self, h: ArenaHandle) -> PseudoNode     { self.pseudo_nodes    .take(h) }
    pub fn take_dummy_node     (&mut self, h: ArenaHandle) -> DummyNode      { self.dummy_nodes     .take(h) }
    pub fn take_opr_node       (&mut self, h: ArenaHandle) -> OprNode        { self.opr_nodes       .take(h) }

    pub fn replace_back_asm_node       (&mut self, h: ArenaHandle, v: AsmNode      ) { self.asm_nodes       .replace_back(h, v) }
    pub fn replace_back_state_node     (&mut self, h: ArenaHandle, v: StateNode    ) { self.state_nodes     .replace_back(h, v) }
    pub fn replace_back_syn_node       (&mut self, h: ArenaHandle, v: SynNode      ) { self.syn_nodes       .replace_back(h, v) }
    pub fn replace_back_wait_cond_node (&mut self, h: ArenaHandle, v: WaitCondNode ) { self.wait_cond_nodes .replace_back(h, v) }
    pub fn replace_back_wait_cycle_node(&mut self, h: ArenaHandle, v: WaitCycleNode) { self.wait_cycle_nodes.replace_back(h, v) }
    pub fn replace_back_counter_node   (&mut self, h: ArenaHandle, v: CounterNode  ) { self.counter_nodes   .replace_back(h, v) }
    pub fn replace_back_pseudo_node    (&mut self, h: ArenaHandle, v: PseudoNode   ) { self.pseudo_nodes    .replace_back(h, v) }
    pub fn replace_back_dummy_node     (&mut self, h: ArenaHandle, v: DummyNode    ) { self.dummy_nodes     .replace_back(h, v) }
    pub fn replace_back_opr_node       (&mut self, h: ArenaHandle, v: OprNode      ) { self.opr_nodes       .replace_back(h, v) }

    // ----- trait-object dispatch ------------------------------------------
    pub fn get_ncp_node    (&self,     ident: &NcpIdent) -> &    dyn NcpNode { dispatch_ncp!(self, ident, get    ) }
    pub fn get_ncp_node_mut(&mut self, ident: &NcpIdent) -> &mut dyn NcpNode { dispatch_ncp!(self, ident, get_mut) }

    pub fn get_node_exit_opr       (&self, ident: &NcpIdent) -> HcpIdent { self.get_ncp_node(ident).get_exit_opr() }
    pub fn get_node_state_operating(&self, ident: &NcpIdent) -> HcpIdent { self.get_ncp_node(ident).get_state_operating() }
}
