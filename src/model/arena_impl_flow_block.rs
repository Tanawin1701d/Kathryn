use crate::model::common::identifier::Identifiable;
use crate::model::flow_block::{FlowBlock, FlowBlockIdent, FlowBlockPar, FlowBlockSeq, FlowBlockType};
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

    // --- par (unified) ---

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

    // --- polymorphic take / replace / get ---

    pub fn take_flow_block(&mut self, ident: FlowBlockIdent) -> Box<dyn FlowBlock> {
        match ident.get_block_type() {
            FlowBlockType::Sequential => Box::new(self.take_flow_block_seq(ident)),
            FlowBlockType::Parallel   => Box::new(self.take_flow_block_par(ident)),
        }
    }

    pub fn replace_back_flow_block(&mut self, block: Box<dyn FlowBlock>) {
        block.replace_back_into_arena(self);
    }

    pub fn get_flow_block(&self, ident: FlowBlockIdent) -> &dyn FlowBlock {
        match ident.get_block_type() {
            FlowBlockType::Sequential => self.flow_block_seqs.get(*ident.get_arena_handle()),
            FlowBlockType::Parallel   => self.flow_block_pars.get(*ident.get_arena_handle()),
        }
    }
}
