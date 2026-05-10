use crate::model::flow_block::{
    FlowBlockIdent, FlowBlockParAuto, FlowBlockParNoSync, FlowBlockSeq,
};
use crate::model::model_arena::ModelArena;

impl ModelArena {
    pub fn make_flow_block_seq(&mut self, name: &str) -> FlowBlockIdent {
        self.add_flow_block_seq(FlowBlockSeq::new(false, name))
    }

    pub fn mk_flow_block_seq(&mut self, name: &str) -> FlowBlockIdent {
        self.add_flow_block_seq(FlowBlockSeq::new(true, name))
    }

    pub fn make_flow_block_par_auto(&mut self, name: &str) -> FlowBlockIdent {
        self.add_flow_block_par_auto(FlowBlockParAuto::new(false, name))
    }

    pub fn mk_flow_block_par_auto(&mut self, name: &str) -> FlowBlockIdent {
        self.add_flow_block_par_auto(FlowBlockParAuto::new(true, name))
    }

    pub fn make_flow_block_par_no_sync(&mut self, name: &str) -> FlowBlockIdent {
        self.add_flow_block_par_no_sync(FlowBlockParNoSync::new(false, name))
    }

    pub fn mk_flow_block_par_no_sync(&mut self, name: &str) -> FlowBlockIdent {
        self.add_flow_block_par_no_sync(FlowBlockParNoSync::new(true, name))
    }
}
