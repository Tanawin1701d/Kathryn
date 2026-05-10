use crate::model::common::identifier::Identifiable;
use crate::model::flow_block::{FlowBlock, FlowBlockIdent, FlowBlockParAuto, FlowBlockParNoSync, FlowBlockSeq, FlowBlockType};
use crate::model::model_arena::ModelArena;

impl ModelArena {
    pub fn add_flow_block_seq(&mut self, block: FlowBlockSeq) -> FlowBlockIdent {
        let h = self.flow_block_seqs.insert(block);
        self.flow_block_seqs.get(h).get_base().get_ident()
    }

    pub fn add_flow_block_par_auto(&mut self, block: FlowBlockParAuto) -> FlowBlockIdent {
        let h = self.flow_block_par_autos.insert(block);
        self.flow_block_par_autos.get(h).get_base().get_ident()
    }

    pub fn add_flow_block_par_no_sync(&mut self, block: FlowBlockParNoSync) -> FlowBlockIdent {
        let h = self.flow_block_par_no_syncs.insert(block);
        self.flow_block_par_no_syncs.get(h).get_base().get_ident()
    }

    pub fn take_flow_block_seq(&mut self, ident: FlowBlockIdent) -> FlowBlockSeq {
        assert_eq!(ident.get_block_type(), FlowBlockType::Sequential);
        self.flow_block_seqs.take(*ident.get_arena_handle())
    }

    pub fn take_flow_block_par_auto(&mut self, ident: FlowBlockIdent) -> FlowBlockParAuto {
        assert_eq!(ident.get_block_type(), FlowBlockType::ParallelAutoSync);
        self.flow_block_par_autos.take(*ident.get_arena_handle())
    }

    pub fn take_flow_block_par_no_sync(&mut self, ident: FlowBlockIdent) -> FlowBlockParNoSync {
        assert_eq!(ident.get_block_type(), FlowBlockType::ParallelNoSync);
        self.flow_block_par_no_syncs.take(*ident.get_arena_handle())
    }

    pub fn replace_back_flow_block_seq(&mut self, block: FlowBlockSeq) {
        let h = *block.get_arena_handle();
        self.flow_block_seqs.replace_back(h, block);
    }

    pub fn replace_back_flow_block_par_auto(&mut self, block: FlowBlockParAuto) {
        let h = *block.get_arena_handle();
        self.flow_block_par_autos.replace_back(h, block);
    }

    pub fn replace_back_flow_block_par_no_sync(&mut self, block: FlowBlockParNoSync) {
        let h = *block.get_arena_handle();
        self.flow_block_par_no_syncs.replace_back(h, block);
    }

    // --- polymorphic take / replace / get ---

    pub fn take_flow_block(&mut self, ident: FlowBlockIdent) -> Box<dyn FlowBlock> {
        match ident.get_block_type() {
            FlowBlockType::Sequential      => Box::new(self.take_flow_block_seq(ident)),
            FlowBlockType::ParallelAutoSync => Box::new(self.take_flow_block_par_auto(ident)),
            FlowBlockType::ParallelNoSync   => Box::new(self.take_flow_block_par_no_sync(ident)),
        }
    }

    pub fn replace_back_flow_block(&mut self, block: Box<dyn FlowBlock>) {
        block.replace_back_into_arena(self);
    }

    pub fn get_flow_block(&self, ident: FlowBlockIdent) -> &dyn FlowBlock {
        match ident.get_block_type() {
            FlowBlockType::Sequential      => self.flow_block_seqs.get(*ident.get_arena_handle()),
            FlowBlockType::ParallelAutoSync => self.flow_block_par_autos.get(*ident.get_arena_handle()),
            FlowBlockType::ParallelNoSync   => self.flow_block_par_no_syncs.get(*ident.get_arena_handle()),
        }
    }
}
