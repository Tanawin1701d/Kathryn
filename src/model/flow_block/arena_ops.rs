use crate::model::flow_block::{ExtSigType, FlowBlockIdent, NodeWrap};
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::NcpIdent;

impl ModelArena {
    pub fn add_node_to_flow_block(&mut self, block_ident: FlowBlockIdent, node: NcpIdent) {
        let mut block = self.take_flow_block(block_ident);
        block.add_element_in_flow_block(node);
        self.replace_back_flow_block(block);
    }

    pub fn add_sub_flow_block_to_flow_block(
        &mut self,
        parent: FlowBlockIdent,
        child: FlowBlockIdent,
    ) {
        let mut block = self.take_flow_block(parent);
        block.add_sub_flow_block(child);
        self.replace_back_flow_block(block);
    }

    pub fn add_int_signal_to_flow_block(
        &mut self,
        block_ident: FlowBlockIdent,
        int_type: ExtSigType,
        signal: HcpIdent,
    ) {
        let mut block = self.take_flow_block(block_ident);
        block.get_base_mut().add_int_signal(int_type, signal);
        self.replace_back_flow_block(block);
    }

    pub fn add_hold_signal_to_flow_block(&mut self, block_ident: FlowBlockIdent, signal: HcpIdent) {
        let mut block = self.take_flow_block(block_ident);
        block.get_base_mut().add_hold_signal(signal);
        self.replace_back_flow_block(block);
    }

    pub fn build_flow_block(&mut self, ident: FlowBlockIdent) {
        let mut block = self.take_flow_block(ident);
        block.build_hw_master(self);
        self.replace_back_flow_block(block);
    }

    pub fn summarize_flow_block(&self, ident: FlowBlockIdent) -> NodeWrap {
        self.get_flow_block(ident).summarize_block()
    }
}
