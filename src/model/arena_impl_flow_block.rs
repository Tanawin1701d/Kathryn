use crate::model::common::identifier::Identifiable;
use crate::model::flow_block::{
    FlowBlock, FlowBlockIdent, FlowBlockType,
    FlowBlockSeq, FlowBlockPar,
    FlowBlockCond, FlowBlockCondElif,
    FlowBlockZeroCondIf, FlowBlockZeroCondElif,
    FlowBlockWhile, FlowBlockDoWhile, FlowBlockCounterLoop,
};
use crate::model::model_arena::ModelArena;

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

    // --- polymorphic take / replace / get ---

    pub fn take_flow_block(&mut self, ident: FlowBlockIdent) -> Box<dyn FlowBlock> {
        match ident.get_block_type() {
            FlowBlockType::Sequential   => Box::new(self.take_flow_block_seq       (ident)),
            FlowBlockType::Parallel     => Box::new(self.take_flow_block_par       (ident)),
            FlowBlockType::CondIf       => Box::new(self.take_flow_block_cond      (ident)),
            FlowBlockType::CondElif     => Box::new(self.take_flow_block_cond_elif (ident)),
            FlowBlockType::ZeroCondIf   => Box::new(self.take_flow_block_zero_cond_if  (ident)),
            FlowBlockType::ZeroCondElif => Box::new(self.take_flow_block_zero_cond_elif(ident)),
            FlowBlockType::WhileLoop    => Box::new(self.take_flow_block_while     (ident)),
            FlowBlockType::DoWhile      => Box::new(self.take_flow_block_do_while  (ident)),
            FlowBlockType::CounterLoop  => Box::new(self.take_flow_block_counter_loop(ident)),
        }
    }

    pub fn replace_back_flow_block(&mut self, block: Box<dyn FlowBlock>) {
        block.replace_back_into_arena(self);
    }

    pub fn get_flow_block(&self, ident: FlowBlockIdent) -> &dyn FlowBlock {
        match ident.get_block_type() {
            FlowBlockType::Sequential   => self.flow_block_seqs          .get(*ident.get_arena_handle()),
            FlowBlockType::Parallel     => self.flow_block_pars          .get(*ident.get_arena_handle()),
            FlowBlockType::CondIf       => self.flow_block_conds         .get(*ident.get_arena_handle()),
            FlowBlockType::CondElif     => self.flow_block_cond_elifs    .get(*ident.get_arena_handle()),
            FlowBlockType::ZeroCondIf   => self.flow_block_zero_cond_ifs  .get(*ident.get_arena_handle()),
            FlowBlockType::ZeroCondElif => self.flow_block_zero_cond_elifs.get(*ident.get_arena_handle()),
            FlowBlockType::WhileLoop    => self.flow_block_whiles        .get(*ident.get_arena_handle()),
            FlowBlockType::DoWhile      => self.flow_block_do_whiles     .get(*ident.get_arena_handle()),
            FlowBlockType::CounterLoop  => self.flow_block_counter_loops .get(*ident.get_arena_handle()),
        }
    }
}
