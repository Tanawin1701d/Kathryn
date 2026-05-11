use crate::model::common::identifier::Identifiable;
use crate::model::controller::clock_mode::{ClockMode, get_global_clk_mode};
use crate::model::flow_block::flow_block_ident::{FlowBlockIdent, FlowBlockType};
use crate::model::flow_block::node_wrap::NodeWrap;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::{NcpIdent, NodeType};
use crate::model::nodes::node_trigger::NodeTrigger;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
#[repr(usize)]
pub enum ExtSigType {
    Hold  = 0,
    Reset = 1,
    Start = 2,
    MReset  = 3,
}

impl ExtSigType {
    pub const COUNT: usize = 4;
}

#[derive(Clone, Debug)]
pub struct FlowBlockBase {
    // identifier
    ident             : FlowBlockIdent,
    // blocks
    sub_blocks_i      : Vec<FlowBlockIdent>,
    sub_block_orders  : Vec<usize>,
    con_blocks_i      : Vec<FlowBlockIdent>,
    con_block_orders  : Vec<usize>,
    basic_nodes_i     : Vec<NcpIdent>,
    basic_node_orders : Vec<usize>,
    next_input_order  : usize,
    // external signal
    sys_nodes         : Vec<NcpIdent>,
    ext_signals       : [Vec<HcpIdent>; ExtSigType::COUNT],
    ext_trigger_node  : NodeTrigger,


}

impl FlowBlockBase {
    pub fn new(block_type: FlowBlockType, name: &str) -> Self {
        Self {
            // identifier
            ident            : FlowBlockIdent::new(block_type, name),
            // blocks
            sub_blocks_i     : Vec::new(),
            sub_block_orders : Vec::new(),
            con_blocks_i     : Vec::new(),
            con_block_orders : Vec::new(),
            basic_nodes_i    : Vec::new(),
            basic_node_orders: Vec::new(),
            next_input_order : 0,
            // external signal
            sys_nodes        : Vec::new(),
            ext_signals      : [Vec::new(), Vec::new(), Vec::new(), Vec::new()],
            ext_trigger_node : NodeTrigger::default(),
        }
    }

    // get function

    pub fn get_ident    (&self)     -> FlowBlockIdent    { self.ident }
    pub fn get_ident_ref(&self)     -> &FlowBlockIdent   { &self.ident }
    pub fn get_ident_mut(&mut self) -> &mut FlowBlockIdent { &mut self.ident }
    pub fn get_clock_mode(&self)    -> ClockMode         { get_global_clk_mode() }

    pub fn get_sub_blocks_i (&self) -> &[FlowBlockIdent] { &self.sub_blocks_i  }
    pub fn get_con_blocks_i (&self) -> &[FlowBlockIdent] { &self.con_blocks_i  }
    pub fn get_basic_nodes_i(&self) -> &[NcpIdent]       { &self.basic_nodes_i }
    pub fn get_sys_nodes    (&self) -> &[NcpIdent]       { &self.sys_nodes }

    // add block and node
    pub fn add_basic_node(&mut self, node: NcpIdent) {
        assert_eq!(
            node.get_node_type(),
            NodeType::Asm,
            "flow-block basic nodes must be AsmNode"
        );
        self.basic_nodes_i.push(node);
        self.basic_node_orders.push(self.next_input_order);
        self.next_input_order += 1;
    }

    pub fn add_sys_node(&mut self, node: NcpIdent) {
        self.sys_nodes.push(node);
    }

    pub fn add_sub_flow_block(&mut self, block: FlowBlockIdent) {
        self.sub_blocks_i.push(block);
        self.sub_block_orders.push(self.next_input_order);
        self.next_input_order += 1;
    }

    pub fn add_con_flow_block(&mut self, block: FlowBlockIdent) {
        self.con_blocks_i.push(block);
        self.con_block_orders.push(self.next_input_order);
        self.next_input_order += 1;
    }

    pub fn add_int_signal(&mut self, int_type: ExtSigType, signal: HcpIdent) {
        self.ext_signals[int_type as usize].push(signal);
    }

    // --- ext_trigger_node getters ---

    pub fn get_hold_node     (&self) -> Option<NcpIdent> { self.ext_trigger_node.hold_node_i }
    pub fn get_int_reset_node(&self) -> Option<NcpIdent> { self.ext_trigger_node.int_reset_node_i }
    pub fn get_int_start_node(&self) -> Option<NcpIdent> { self.ext_trigger_node.int_start_node_i }
    pub fn get_mrst_node     (&self) -> Option<NcpIdent> { self.ext_trigger_node.mrst_node_i }
    pub fn has_int_start     (&self) -> bool             { self.ext_trigger_node.int_start_node_i.is_some() }

    pub fn get_ext_trigger_node(&self) -> &NodeTrigger { &self.ext_trigger_node }

    pub fn get_int_node(&self, sig_type: ExtSigType) -> Option<NcpIdent> {
        match sig_type {
            ExtSigType::Hold   => self.ext_trigger_node.hold_node_i,
            ExtSigType::Reset  => self.ext_trigger_node.int_reset_node_i,
            ExtSigType::Start  => self.ext_trigger_node.int_start_node_i,
            ExtSigType::MReset => self.ext_trigger_node.mrst_node_i,
        }
    }

    // --- build ---

    fn gen_trigger_node(&mut self, arena: &mut ModelArena) {
        let id = self.ident.get_global_id();
        self.ext_trigger_node.hold_node_i      = Self::gen_signal_node(&format!("flow_hold_{}", id),  &self.ext_signals[ExtSigType::Hold   as usize], arena);
        self.ext_trigger_node.int_reset_node_i = Self::gen_signal_node(&format!("flow_rst_{}", id),   &self.ext_signals[ExtSigType::Reset  as usize], arena);
        self.ext_trigger_node.int_start_node_i = Self::gen_signal_node(&format!("flow_start_{}", id), &self.ext_signals[ExtSigType::Start  as usize], arena);
        self.ext_trigger_node.mrst_node_i      = Self::gen_signal_node(&format!("flow_mrst_{}", id),  &self.ext_signals[ExtSigType::MReset as usize], arena);
    }

    fn gen_signal_node(name: &str, signals: &[HcpIdent], arena: &mut ModelArena) -> Option<NcpIdent> {
        let mut iter = signals.iter().copied();
        let mut sig = iter.next()?;
        for rhs in iter {
            sig = arena.make_expression(name, LogicOp::BitwiseOr, sig, rhs, None, None);
        }
        Some(arena.make_opr_node(name, sig))
    }

    // pre build hardware function
    pub fn build_common_hw(&mut self, arena: &mut ModelArena) {
        for sig_type in [ExtSigType::Reset, ExtSigType::Hold, ExtSigType::MReset] {
            self.fill_ext_signal_to_child(arena, sig_type);
        }
        self.build_sub_hw_component(arena);
        self.gen_trigger_node(arena);
    }

    fn fill_ext_signal_to_child(&self, arena: &mut ModelArena, sig_type: ExtSigType) {
        for signal in &self.ext_signals[sig_type as usize] {
            for child in self.sub_blocks_i.iter().chain(self.con_blocks_i.iter()) {
                arena.add_ext_signal_to_flow_block(*child, sig_type, *signal);
            }
        }
    }

    fn build_sub_hw_component(&self, arena: &mut ModelArena) {
        for child in self.sub_blocks_i.iter().chain(self.con_blocks_i.iter()) {
            arena.build_flow_block(*child);
        }
    }

}

pub trait FlowBlock: Identifiable {
    // access the base element
    fn get_base(&self) -> &FlowBlockBase;
    fn get_base_mut(&mut self) -> &mut FlowBlockBase;
    // ownership management
    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena);
    // add element
    fn add_element_in_flow_block(&mut self, node: NcpIdent);
    fn add_sub_flow_block(&mut self, block: FlowBlockIdent);
    fn add_con_flow_block(&mut self, _block: FlowBlockIdent) {
        panic!("add_con_flow_block: not supported by this block type")
    }
    // conditional / loop queries
    fn get_con_condition(&self) -> Option<HcpIdent> {
        panic!("get_con_condition: not supported by this block type")
    }
    fn get_loop_id_expr_i(&self) -> HcpIdent {
        panic!("get_loop_id_expr_i: not supported by this block type")
    }
    // summarize for next synthesis
    fn summarize_block(&self) -> NodeWrap;
    // main build core
    fn build_hw_component(&mut self, arena: &mut ModelArena);
    fn build_hw_master(&mut self, arena: &mut ModelArena) {
        self.get_base_mut().build_common_hw(arena);
        self.build_hw_component(arena);
    }
}
