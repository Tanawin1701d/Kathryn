use crate::model::flow_block::{
    FlowBlockIdent,
    FlowBlockSeq, FlowBlockPar,
    FlowBlockCond, FlowBlockCondElif,
    FlowBlockZeroCond,
    FlowBlockWhile, FlowBlockDoWhile, FlowBlockCounterLoop,
};
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::model_arena::ModelArena;

impl ModelArena {
    // ---- seq ----------------------------------------------------------------

    pub fn make_flow_block_seq(&mut self, name: &str) -> FlowBlockIdent {
        self.add_flow_block_seq(FlowBlockSeq::new(name))
    }
    pub fn mk_flow_block_seq(&mut self, name: &str) -> FlowBlockIdent {
        self.add_flow_block_seq(FlowBlockSeq::new(name))
    }

    // ---- par ----------------------------------------------------------------

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

    // ---- cond: CIF ----------------------------------------------------------

    pub fn make_flow_block_cif(&mut self, name: &str, cond_i: HcpIdent) -> FlowBlockIdent {
        self.add_flow_block_cond(FlowBlockCond::new_cif(name, cond_i))
    }
    pub fn mk_flow_block_cif(&mut self, name: &str, cond_i: HcpIdent) -> FlowBlockIdent {
        self.add_flow_block_cond(FlowBlockCond::new_cif(name, cond_i))
    }

    // ---- cond: SIF ----------------------------------------------------------

    pub fn make_flow_block_sif(&mut self, name: &str, cond_i: HcpIdent) -> FlowBlockIdent {
        self.add_flow_block_cond(FlowBlockCond::new_sif(name, cond_i))
    }
    pub fn mk_flow_block_sif(&mut self, name: &str, cond_i: HcpIdent) -> FlowBlockIdent {
        self.add_flow_block_cond(FlowBlockCond::new_sif(name, cond_i))
    }

    // ---- cond_elif: CSELIF --------------------------------------------------

    pub fn make_flow_block_cselif(&mut self, name: &str, cond_i: HcpIdent) -> FlowBlockIdent {
        self.add_flow_block_cond_elif(FlowBlockCondElif::new_elif(name, cond_i))
    }
    pub fn mk_flow_block_cselif(&mut self, name: &str, cond_i: HcpIdent) -> FlowBlockIdent {
        self.add_flow_block_cond_elif(FlowBlockCondElif::new_elif(name, cond_i))
    }

    // ---- cond_elif: CSELSE --------------------------------------------------

    pub fn make_flow_block_cselse(&mut self, name: &str) -> FlowBlockIdent {
        self.add_flow_block_cond_elif(FlowBlockCondElif::new_else(name))
    }
    pub fn mk_flow_block_cselse(&mut self, name: &str) -> FlowBlockIdent {
        self.add_flow_block_cond_elif(FlowBlockCondElif::new_else(name))
    }

    // ---- zero_cond: ZIF -----------------------------------------------------

    pub fn make_flow_block_zif(&mut self, name: &str, cond_i: HcpIdent) -> FlowBlockIdent {
        self.add_flow_block_zero_cond(FlowBlockZeroCond::new_zif(name, cond_i))
    }
    pub fn mk_flow_block_zif(&mut self, name: &str, cond_i: HcpIdent) -> FlowBlockIdent {
        self.add_flow_block_zero_cond(FlowBlockZeroCond::new_zif(name, cond_i))
    }

    // ---- zero_cond: ZELIF ---------------------------------------------------

    pub fn make_flow_block_zelif(&mut self, name: &str, cond_i: HcpIdent) -> FlowBlockIdent {
        self.add_flow_block_zero_cond(FlowBlockZeroCond::new_zelif(name, cond_i))
    }
    pub fn mk_flow_block_zelif(&mut self, name: &str, cond_i: HcpIdent) -> FlowBlockIdent {
        self.add_flow_block_zero_cond(FlowBlockZeroCond::new_zelif(name, cond_i))
    }

    // ---- zero_cond: ZELSE ---------------------------------------------------

    pub fn make_flow_block_zelse(&mut self, name: &str) -> FlowBlockIdent {
        self.add_flow_block_zero_cond(FlowBlockZeroCond::new_zelse(name))
    }
    pub fn mk_flow_block_zelse(&mut self, name: &str) -> FlowBlockIdent {
        self.add_flow_block_zero_cond(FlowBlockZeroCond::new_zelse(name))
    }

    // ---- while: CWHILE ------------------------------------------------------

    pub fn make_flow_block_cwhile(&mut self, name: &str, cond_i: HcpIdent) -> FlowBlockIdent {
        self.add_flow_block_while(FlowBlockWhile::new_cwhile(name, cond_i))
    }
    pub fn mk_flow_block_cwhile(&mut self, name: &str, cond_i: HcpIdent) -> FlowBlockIdent {
        self.add_flow_block_while(FlowBlockWhile::new_cwhile(name, cond_i))
    }

    // ---- while: SWHILE ------------------------------------------------------

    pub fn make_flow_block_swhile(&mut self, name: &str, cond_i: HcpIdent) -> FlowBlockIdent {
        self.add_flow_block_while(FlowBlockWhile::new_swhile(name, cond_i))
    }
    pub fn mk_flow_block_swhile(&mut self, name: &str, cond_i: HcpIdent) -> FlowBlockIdent {
        self.add_flow_block_while(FlowBlockWhile::new_swhile(name, cond_i))
    }

    // ---- do_while -----------------------------------------------------------

    pub fn make_flow_block_do_while(&mut self, name: &str, cond_i: HcpIdent) -> FlowBlockIdent {
        self.add_flow_block_do_while(FlowBlockDoWhile::new(name, cond_i))
    }
    pub fn mk_flow_block_do_while(&mut self, name: &str, cond_i: HcpIdent) -> FlowBlockIdent {
        self.add_flow_block_do_while(FlowBlockDoWhile::new(name, cond_i))
    }

    // ---- counter_loop -------------------------------------------------------

    pub fn make_flow_block_counter_loop(&mut self, name: &str, last_loop_cnt: i32) -> FlowBlockIdent {
        self.add_flow_block_counter_loop(FlowBlockCounterLoop::new(name, last_loop_cnt))
    }
    pub fn mk_flow_block_counter_loop(&mut self, name: &str, last_loop_cnt: i32) -> FlowBlockIdent {
        self.add_flow_block_counter_loop(FlowBlockCounterLoop::new(name, last_loop_cnt))
    }
}
