use crate::model::flow_block::{FlowBlockIdent, FlowBlockPar, FlowBlockSeq};
use crate::model::model_arena::ModelArena;

impl ModelArena {
    pub fn make_flow_block_seq(&mut self, name: &str) -> FlowBlockIdent {
        self.add_flow_block_seq(FlowBlockSeq::new(name))
    }
    pub fn mk_flow_block_seq(&mut self, name: &str) -> FlowBlockIdent {
        self.add_flow_block_seq(FlowBlockSeq::new(name))
    }

    pub fn make_flow_block_par_auto(&mut self, name: &str) -> FlowBlockIdent {
        self.add_flow_block_par(FlowBlockPar::new_auto_sync(name))
    }
    pub fn mk_flow_block_par_auto(&mut self, name: &str) -> FlowBlockIdent {
        self.add_flow_block_par(FlowBlockPar::new_auto_sync(name))
    }

    pub fn make_flow_block_par_no_sync(&mut self, name: &str) -> FlowBlockIdent {
        self.add_flow_block_par(FlowBlockPar::new_no_sync(name))
    }
    pub fn mk_flow_block_par_no_sync(&mut self, name: &str) -> FlowBlockIdent {
        self.add_flow_block_par(FlowBlockPar::new_no_sync(name))
    }
}
