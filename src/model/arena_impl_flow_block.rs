use crate::model::common::identifier::Identifiable;
use crate::model::flow_block::{
    ExtSigType, FlowBlock, FlowBlockIdent, FlowBlockType, NodeWrap,
    FlowBlockSeq, FlowBlockPar,
    FlowBlockCond, FlowBlockCondElif,
    FlowBlockZeroCondIf, FlowBlockZeroCondElif,
    FlowBlockZeroSwitch, FlowBlockZeroSwitchCase,
    FlowBlockWhile, FlowBlockDoWhile, FlowBlockCounterLoop,
    FlowBlockWait,
};
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::NcpIdent;

impl ModelArena {
    // --- seq ---

    pub fn add_flow_block_seq(&mut self, block: FlowBlockSeq) -> FlowBlockIdent {
        let h = self.flow_block_seqs.insert(block);
        self.flow_block_seqs.get(h).get_base().get_ident()
    }

    pub fn take_flow_block_seq(&mut self, ident: FlowBlockIdent) -> FlowBlockSeq {
        assert_eq!(ident.get_block_type(), FlowBlockType::Sequential);
        self.flow_block_seqs.take(*ident.get_arena_handle())
    }

    pub fn replace_back_flow_block_seq(&mut self, block: FlowBlockSeq) {
        let h = *block.get_arena_handle();
        self.flow_block_seqs.replace_back(h, block);
    }

    // --- par ---

    pub fn add_flow_block_par(&mut self, block: FlowBlockPar) -> FlowBlockIdent {
        let h = self.flow_block_pars.insert(block);
        self.flow_block_pars.get(h).get_base().get_ident()
    }

    pub fn take_flow_block_par(&mut self, ident: FlowBlockIdent) -> FlowBlockPar {
        assert_eq!(ident.get_block_type(), FlowBlockType::Parallel);
        self.flow_block_pars.take(*ident.get_arena_handle())
    }

    pub fn replace_back_flow_block_par(&mut self, block: FlowBlockPar) {
        let h = *block.get_arena_handle();
        self.flow_block_pars.replace_back(h, block);
    }

    // --- cond (CIF / SIF) ---

    pub fn add_flow_block_cond(&mut self, block: FlowBlockCond) -> FlowBlockIdent {
        let h = self.flow_block_conds.insert(block);
        self.flow_block_conds.get(h).get_base().get_ident()
    }

    pub fn take_flow_block_cond(&mut self, ident: FlowBlockIdent) -> FlowBlockCond {
        assert_eq!(ident.get_block_type(), FlowBlockType::CondIf);
        self.flow_block_conds.take(*ident.get_arena_handle())
    }

    pub fn replace_back_flow_block_cond(&mut self, block: FlowBlockCond) {
        let h = *block.get_arena_handle();
        self.flow_block_conds.replace_back(h, block);
    }

    // --- cond_elif (CSELIF / CSELSE) ---

    pub fn add_flow_block_cond_elif(&mut self, block: FlowBlockCondElif) -> FlowBlockIdent {
        let h = self.flow_block_cond_elifs.insert(block);
        self.flow_block_cond_elifs.get(h).get_base().get_ident()
    }

    pub fn take_flow_block_cond_elif(&mut self, ident: FlowBlockIdent) -> FlowBlockCondElif {
        assert_eq!(ident.get_block_type(), FlowBlockType::CondElif);
        self.flow_block_cond_elifs.take(*ident.get_arena_handle())
    }

    pub fn replace_back_flow_block_cond_elif(&mut self, block: FlowBlockCondElif) {
        let h = *block.get_arena_handle();
        self.flow_block_cond_elifs.replace_back(h, block);
    }

    // --- zero_cond_if (ZIF master) ---

    pub fn add_flow_block_zero_cond_if(&mut self, block: FlowBlockZeroCondIf) -> FlowBlockIdent {
        let h = self.flow_block_zero_cond_ifs.insert(block);
        self.flow_block_zero_cond_ifs.get(h).get_base().get_ident()
    }

    pub fn take_flow_block_zero_cond_if(&mut self, ident: FlowBlockIdent) -> FlowBlockZeroCondIf {
        assert_eq!(ident.get_block_type(), FlowBlockType::ZeroCondIf);
        self.flow_block_zero_cond_ifs.take(*ident.get_arena_handle())
    }

    pub fn replace_back_flow_block_zero_cond_if(&mut self, block: FlowBlockZeroCondIf) {
        let h = *block.get_arena_handle();
        self.flow_block_zero_cond_ifs.replace_back(h, block);
    }

    // --- zero_cond_elif (ZELIF / ZELSE) ---

    pub fn add_flow_block_zero_cond_elif(&mut self, block: FlowBlockZeroCondElif) -> FlowBlockIdent {
        let h = self.flow_block_zero_cond_elifs.insert(block);
        self.flow_block_zero_cond_elifs.get(h).get_base().get_ident()
    }

    pub fn take_flow_block_zero_cond_elif(&mut self, ident: FlowBlockIdent) -> FlowBlockZeroCondElif {
        assert_eq!(ident.get_block_type(), FlowBlockType::ZeroCondElif);
        self.flow_block_zero_cond_elifs.take(*ident.get_arena_handle())
    }

    pub fn replace_back_flow_block_zero_cond_elif(&mut self, block: FlowBlockZeroCondElif) {
        let h = *block.get_arena_handle();
        self.flow_block_zero_cond_elifs.replace_back(h, block);
    }

    // --- zero_switch (master) ---

    pub fn add_flow_block_zero_switch(&mut self, block: FlowBlockZeroSwitch) -> FlowBlockIdent {
        let h = self.flow_block_zero_switches.insert(block);
        self.flow_block_zero_switches.get(h).get_base().get_ident()
    }

    pub fn take_flow_block_zero_switch(&mut self, ident: FlowBlockIdent) -> FlowBlockZeroSwitch {
        assert_eq!(ident.get_block_type(), FlowBlockType::ZeroSwitch);
        self.flow_block_zero_switches.take(*ident.get_arena_handle())
    }

    pub fn replace_back_flow_block_zero_switch(&mut self, block: FlowBlockZeroSwitch) {
        let h = *block.get_arena_handle();
        self.flow_block_zero_switches.replace_back(h, block);
    }

    // --- zero_switch_case ---

    pub fn add_flow_block_zero_switch_case(&mut self, block: FlowBlockZeroSwitchCase) -> FlowBlockIdent {
        let h = self.flow_block_zero_switch_cases.insert(block);
        self.flow_block_zero_switch_cases.get(h).get_base().get_ident()
    }

    pub fn take_flow_block_zero_switch_case(&mut self, ident: FlowBlockIdent) -> FlowBlockZeroSwitchCase {
        assert_eq!(ident.get_block_type(), FlowBlockType::ZeroSwitchCase);
        self.flow_block_zero_switch_cases.take(*ident.get_arena_handle())
    }

    pub fn replace_back_flow_block_zero_switch_case(&mut self, block: FlowBlockZeroSwitchCase) {
        let h = *block.get_arena_handle();
        self.flow_block_zero_switch_cases.replace_back(h, block);
    }

    // --- while (CWHILE / SWHILE) ---

    pub fn add_flow_block_while(&mut self, block: FlowBlockWhile) -> FlowBlockIdent {
        let h = self.flow_block_whiles.insert(block);
        self.flow_block_whiles.get(h).get_base().get_ident()
    }

    pub fn take_flow_block_while(&mut self, ident: FlowBlockIdent) -> FlowBlockWhile {
        assert_eq!(ident.get_block_type(), FlowBlockType::WhileLoop);
        self.flow_block_whiles.take(*ident.get_arena_handle())
    }

    pub fn replace_back_flow_block_while(&mut self, block: FlowBlockWhile) {
        let h = *block.get_arena_handle();
        self.flow_block_whiles.replace_back(h, block);
    }

    // --- do_while ---

    pub fn add_flow_block_do_while(&mut self, block: FlowBlockDoWhile) -> FlowBlockIdent {
        let h = self.flow_block_do_whiles.insert(block);
        self.flow_block_do_whiles.get(h).get_base().get_ident()
    }

    pub fn take_flow_block_do_while(&mut self, ident: FlowBlockIdent) -> FlowBlockDoWhile {
        assert_eq!(ident.get_block_type(), FlowBlockType::DoWhile);
        self.flow_block_do_whiles.take(*ident.get_arena_handle())
    }

    pub fn replace_back_flow_block_do_while(&mut self, block: FlowBlockDoWhile) {
        let h = *block.get_arena_handle();
        self.flow_block_do_whiles.replace_back(h, block);
    }

    // --- counter_loop ---

    pub fn add_flow_block_counter_loop(&mut self, block: FlowBlockCounterLoop) -> FlowBlockIdent {
        let h = self.flow_block_counter_loops.insert(block);
        self.flow_block_counter_loops.get(h).get_base().get_ident()
    }

    pub fn take_flow_block_counter_loop(&mut self, ident: FlowBlockIdent) -> FlowBlockCounterLoop {
        assert_eq!(ident.get_block_type(), FlowBlockType::CounterLoop);
        self.flow_block_counter_loops.take(*ident.get_arena_handle())
    }

    pub fn replace_back_flow_block_counter_loop(&mut self, block: FlowBlockCounterLoop) {
        let h = *block.get_arena_handle();
        self.flow_block_counter_loops.replace_back(h, block);
    }

    // --- wait (SCWAIT / SYWAIT) ---

    pub fn add_flow_block_wait(&mut self, block: FlowBlockWait) -> FlowBlockIdent {
        let h = self.flow_block_waits.insert(block);
        self.flow_block_waits.get(h).get_base().get_ident()
    }

    pub fn take_flow_block_wait(&mut self, ident: FlowBlockIdent) -> FlowBlockWait {
        assert_eq!(ident.get_block_type(), FlowBlockType::Wait);
        self.flow_block_waits.take(*ident.get_arena_handle())
    }

    pub fn replace_back_flow_block_wait(&mut self, block: FlowBlockWait) {
        let h = *block.get_arena_handle();
        self.flow_block_waits.replace_back(h, block);
    }

    // --- polymorphic take / replace / get ---

    pub fn take_flow_block(&mut self, ident: FlowBlockIdent) -> Box<dyn FlowBlock> {
        match ident.get_block_type() {
            FlowBlockType::Sequential     => Box::new(self.take_flow_block_seq              (ident)),
            FlowBlockType::Parallel       => Box::new(self.take_flow_block_par              (ident)),
            FlowBlockType::CondIf         => Box::new(self.take_flow_block_cond             (ident)),
            FlowBlockType::CondElif       => Box::new(self.take_flow_block_cond_elif        (ident)),
            FlowBlockType::ZeroCondIf     => Box::new(self.take_flow_block_zero_cond_if     (ident)),
            FlowBlockType::ZeroCondElif   => Box::new(self.take_flow_block_zero_cond_elif   (ident)),
            FlowBlockType::ZeroSwitch     => Box::new(self.take_flow_block_zero_switch      (ident)),
            FlowBlockType::ZeroSwitchCase => Box::new(self.take_flow_block_zero_switch_case (ident)),
            FlowBlockType::WhileLoop      => Box::new(self.take_flow_block_while            (ident)),
            FlowBlockType::DoWhile        => Box::new(self.take_flow_block_do_while         (ident)),
            FlowBlockType::CounterLoop    => Box::new(self.take_flow_block_counter_loop     (ident)),
            FlowBlockType::Wait           => Box::new(self.take_flow_block_wait             (ident)),
        }
    }

    pub fn replace_back_flow_block(&mut self, block: Box<dyn FlowBlock>) {
        block.replace_back_into_arena(self);
    }

}

// ---- higher-level flow-block operations (formerly flow_block/arena_ops.rs) ----

impl ModelArena {
    pub fn add_node_to_flow_block(&mut self, block_ident: FlowBlockIdent, node: NcpIdent) {
        let mut block = self.take_flow_block(block_ident);
        block.add_element_in_flow_block(node);
        self.replace_back_flow_block(block);
    }

    pub fn add_sub_flow_block_to_flow_block(
        &mut self,
        parent: FlowBlockIdent,
        child : FlowBlockIdent,
    ) {
        let mut block = self.take_flow_block(parent);
        block.add_sub_flow_block(child);
        self.replace_back_flow_block(block);
    }

    pub fn add_con_flow_block_to_flow_block(
        &mut self,
        parent: FlowBlockIdent,
        child : FlowBlockIdent,
    ) {
        let mut block = self.take_flow_block(parent);
        block.add_con_flow_block(child);
        self.replace_back_flow_block(block);
    }

    pub fn add_ext_signal_to_flow_block(
        &mut self,
        block_ident: FlowBlockIdent,
        int_type   : ExtSigType,
        signal     : HcpIdent,
    ) {
        let mut block = self.take_flow_block(block_ident);
        block.get_base_mut().add_int_signal(int_type, signal);
        self.replace_back_flow_block(block);
    }

    pub fn build_flow_block(&mut self, ident: FlowBlockIdent) {
        let mut block = self.take_flow_block(ident);
        block.build_hw_master(self);
        self.replace_back_flow_block(block);
    }

    pub fn summarize_flow_block(&mut self, ident: FlowBlockIdent) -> NodeWrap {
        let block = self.take_flow_block(ident);
        let wrap  = block.summarize_as_block();
        self.replace_back_flow_block(block);
        wrap
    }
}
