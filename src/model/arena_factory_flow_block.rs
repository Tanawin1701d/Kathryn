use crate::model::flow_block::{
    FlowBlockIdent,
    FlowBlockSeq, FlowBlockPar,
    FlowBlockCond, FlowBlockCondElif,
    FlowBlockZeroCondIf, FlowBlockZeroCondElif,
    FlowBlockWhile, FlowBlockDoWhile, FlowBlockCounterLoop,
};
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::model_arena::ModelArena;

impl ModelArena {
    // ---- seq ----------------------------------------------------------------

    pub fn make_flow_block_seq(&mut self, name: &str) -> FlowBlockIdent {
        let i = self.add_flow_block_seq(FlowBlockSeq::new(name));
        self.push_flow_block_init_stack(i);
        i
    }

    // ---- par ----------------------------------------------------------------

    pub fn make_flow_block_par_auto(&mut self, name: &str) -> FlowBlockIdent {
        let i = self.add_flow_block_par(FlowBlockPar::new_auto_sync(name));
        self.push_flow_block_init_stack(i);
        i
    }

    pub fn make_flow_block_par_no_sync(&mut self, name: &str) -> FlowBlockIdent {
        let i = self.add_flow_block_par(FlowBlockPar::new_no_sync(name));
        self.push_flow_block_init_stack(i);
        i
    }

    // ---- cond: CIF ----------------------------------------------------------

    pub fn make_flow_block_cif(&mut self, name: &str, cond_i: HcpIdent) -> FlowBlockIdent {
        let i = self.add_flow_block_cond(FlowBlockCond::new_cif(name, cond_i));
        self.push_flow_block_init_stack(i);
        i
    }

    // ---- cond: SIF ----------------------------------------------------------

    pub fn make_flow_block_sif(&mut self, name: &str, cond_i: HcpIdent) -> FlowBlockIdent {
        let i = self.add_flow_block_cond(FlowBlockCond::new_sif(name, cond_i));
        self.push_flow_block_init_stack(i);
        i
    }

    // ---- cond_elif: CSELIF --------------------------------------------------

    pub fn make_flow_block_cselif(&mut self, name: &str, cond_i: HcpIdent) -> FlowBlockIdent {
        let i = self.add_flow_block_cond_elif(FlowBlockCondElif::new_elif(name, cond_i));
        self.push_flow_block_init_stack(i);
        i
    }

    // ---- cond_elif: CSELSE --------------------------------------------------

    pub fn make_flow_block_cselse(&mut self, name: &str) -> FlowBlockIdent {
        let i = self.add_flow_block_cond_elif(FlowBlockCondElif::new_else(name));
        self.push_flow_block_init_stack(i);
        i
    }

    // ---- zero_cond_if: ZIF --------------------------------------------------

    pub fn make_flow_block_zif(&mut self, name: &str, cond_i: HcpIdent) -> FlowBlockIdent {
        let i = self.add_flow_block_zero_cond_if(FlowBlockZeroCondIf::new(name, cond_i));
        self.push_flow_block_init_stack(i);
        i
    }

    // ---- zero_cond_elif: ZELIF ----------------------------------------------

    pub fn make_flow_block_zelif(&mut self, name: &str, cond_i: HcpIdent) -> FlowBlockIdent {
        let i = self.add_flow_block_zero_cond_elif(FlowBlockZeroCondElif::new_zelif(name, cond_i));
        self.push_flow_block_init_stack(i);
        i
    }

    // ---- zero_cond_elif: ZELSE ----------------------------------------------

    pub fn make_flow_block_zelse(&mut self, name: &str) -> FlowBlockIdent {
        let i = self.add_flow_block_zero_cond_elif(FlowBlockZeroCondElif::new_zelse(name));
        self.push_flow_block_init_stack(i);
        i
    }

    // ---- while: CWHILE ------------------------------------------------------

    pub fn make_flow_block_cwhile(&mut self, name: &str, cond_i: HcpIdent) -> FlowBlockIdent {
        let i = self.add_flow_block_while(FlowBlockWhile::new_cwhile(name, cond_i));
        self.push_flow_block_init_stack(i);
        i
    }

    // ---- while: SWHILE ------------------------------------------------------

    pub fn make_flow_block_swhile(&mut self, name: &str, cond_i: HcpIdent) -> FlowBlockIdent {
        let i = self.add_flow_block_while(FlowBlockWhile::new_swhile(name, cond_i));
        self.push_flow_block_init_stack(i);
        i
    }

    // ---- do_while -----------------------------------------------------------

    pub fn make_flow_block_do_while(&mut self, name: &str, cond_i: HcpIdent) -> FlowBlockIdent {
        let i = self.add_flow_block_do_while(FlowBlockDoWhile::new(name, cond_i));
        self.push_flow_block_init_stack(i);
        i
    }

    // ---- counter_loop -------------------------------------------------------

    pub fn make_flow_block_counter_loop(&mut self, name: &str, last_loop_cnt: i32) -> FlowBlockIdent {
        let i = self.add_flow_block_counter_loop(FlowBlockCounterLoop::new(name, last_loop_cnt));
        self.push_flow_block_init_stack(i);
        i
    }
}
