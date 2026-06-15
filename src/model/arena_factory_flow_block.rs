use crate::model::flow_block::{
    FlowBlockIdent,
    FlowBlockSeq, FlowBlockPar,
    FlowBlockCond, FlowBlockCondElif,
    FlowBlockZeroCondIf, FlowBlockZeroCondElif,
    FlowBlockZeroSwitch, FlowBlockZeroSwitchCase,
    FlowBlockWhile, FlowBlockDoWhile, FlowBlockCounterLoop,
    FlowBlockWait,
};
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::hw_component::common::slice::Slice;
use crate::model::model_arena::ModelArena;

impl ModelArena {
    // ---- cond-slice resolution ----------------------------------------------

    /// True when `slice` covers the entire bit-width of HW component `ident`.
    /// A default/invalid slice ({-1,-1}) is treated as "entire".
    pub fn is_slice_cover_entire_hw(&self, ident: &HcpIdent, slice: &Slice) -> bool {
        if !slice.check_valid_slice() { return true; }   // default {-1,-1} = whole signal
        slice.is_contain(&self.get_hw_slice(ident))
    }

    /// Resolve a `(cond_i, slice)` pair to the signal a flow block should gate on.
    /// When `slice` covers `cond_i` entirely (or is absent), `cond_i` is used as-is;
    /// a partial slice is wrapped in a `SliceBit` expression `cond_i[slice]` so the
    /// block gates on exactly the requested bits.
    fn resolve_cond_slice(&mut self, name: &str, cond_i: HcpIdent, cond_slice: Option<Slice>) -> HcpIdent {
        match cond_slice {
            Some(s) if !self.is_slice_cover_entire_hw(&cond_i, &s) =>
                self.make_expression_single(false, &format!("{name}_CONDSLICE"), LogicOp::SliceBit, cond_i, Some(s)),
            _ => cond_i,
        }
    }

    // ---- seq ----------------------------------------------------------------

    pub fn make_flow_block_seq(&mut self, name: &str) -> FlowBlockIdent {
        let i = self.add_flow_block_seq(FlowBlockSeq::new(name));
        i
    }

    // ---- par ----------------------------------------------------------------

    pub fn make_flow_block_par_auto(&mut self, name: &str) -> FlowBlockIdent {
        let i = self.add_flow_block_par(FlowBlockPar::new_auto_sync(name));
        i
    }

    pub fn make_flow_block_par_no_sync(&mut self, name: &str) -> FlowBlockIdent {
        let i = self.add_flow_block_par(FlowBlockPar::new_no_sync(name));
        i
    }

    // ---- cond: CIF ----------------------------------------------------------

    pub fn make_flow_block_cif(&mut self, name: &str, cond_i: HcpIdent, cond_slice: Option<Slice>) -> FlowBlockIdent {
        let cond_i = self.resolve_cond_slice(name, cond_i, cond_slice);
        let i = self.add_flow_block_cond(FlowBlockCond::new_cif(name, cond_i));
        i
    }

    // ---- cond: SIF ----------------------------------------------------------

    pub fn make_flow_block_sif(&mut self, name: &str, cond_i: HcpIdent, cond_slice: Option<Slice>) -> FlowBlockIdent {
        let cond_i = self.resolve_cond_slice(name, cond_i, cond_slice);
        let i = self.add_flow_block_cond(FlowBlockCond::new_sif(name, cond_i));
        i
    }

    // ---- cond_elif: CSELIF --------------------------------------------------

    pub fn make_flow_block_cselif(&mut self, name: &str, cond_i: HcpIdent, cond_slice: Option<Slice>) -> FlowBlockIdent {
        let cond_i = self.resolve_cond_slice(name, cond_i, cond_slice);
        let i = self.add_flow_block_cond_elif(FlowBlockCondElif::new_elif(name, cond_i));
        i
    }

    // ---- cond_elif: CSELSE --------------------------------------------------

    pub fn make_flow_block_cselse(&mut self, name: &str) -> FlowBlockIdent {
        let i = self.add_flow_block_cond_elif(FlowBlockCondElif::new_else(name));
        i
    }

    // ---- zero_cond_if: ZIF --------------------------------------------------

    pub fn make_flow_block_zif(&mut self, name: &str, cond_i: HcpIdent, cond_slice: Option<Slice>) -> FlowBlockIdent {
        let cond_i = self.resolve_cond_slice(name, cond_i, cond_slice);
        let i = self.add_flow_block_zero_cond_if(FlowBlockZeroCondIf::new(name, cond_i));
        i
    }

    // ---- zero_cond_elif: ZELIF ----------------------------------------------

    pub fn make_flow_block_zelif(&mut self, name: &str, cond_i: HcpIdent, cond_slice: Option<Slice>) -> FlowBlockIdent {
        let cond_i = self.resolve_cond_slice(name, cond_i, cond_slice);
        let i = self.add_flow_block_zero_cond_elif(FlowBlockZeroCondElif::new_zelif(name, cond_i));
        i
    }

    // ---- zero_cond_elif: ZELSE ----------------------------------------------

    pub fn make_flow_block_zelse(&mut self, name: &str) -> FlowBlockIdent {
        let i = self.add_flow_block_zero_cond_elif(FlowBlockZeroCondElif::new_zelse(name));
        i
    }

    // ---- zero_switch: ZSTATE ------------------------------------------------

    pub fn make_flow_block_zstate(&mut self, name: &str, state_i: HcpIdent) -> FlowBlockIdent {
        let i = self.add_flow_block_zero_switch(FlowBlockZeroSwitch::new(name, state_i));
        i
    }

    // ---- zero_switch_case: ZCASE --------------------------------------------

    pub fn make_flow_block_zcase(&mut self, name: &str, match_val: i32) -> FlowBlockIdent {
        let i = self.add_flow_block_zero_switch_case(FlowBlockZeroSwitchCase::new(name, match_val));
        i
    }

    // ---- while: CWHILE ------------------------------------------------------

    pub fn make_flow_block_cwhile(&mut self, name: &str, cond_i: HcpIdent, cond_slice: Option<Slice>) -> FlowBlockIdent {
        let cond_i = self.resolve_cond_slice(name, cond_i, cond_slice);
        let i = self.add_flow_block_while(FlowBlockWhile::new_cwhile(name, cond_i));
        i
    }

    // ---- while: SWHILE ------------------------------------------------------

    pub fn make_flow_block_swhile(&mut self, name: &str, cond_i: HcpIdent, cond_slice: Option<Slice>) -> FlowBlockIdent {
        let cond_i = self.resolve_cond_slice(name, cond_i, cond_slice);
        let i = self.add_flow_block_while(FlowBlockWhile::new_swhile(name, cond_i));
        i
    }

    // ---- do_while -----------------------------------------------------------

    pub fn make_flow_block_do_while(&mut self, name: &str, cond_i: HcpIdent, cond_slice: Option<Slice>) -> FlowBlockIdent {
        let cond_i = self.resolve_cond_slice(name, cond_i, cond_slice);
        let i = self.add_flow_block_do_while(FlowBlockDoWhile::new(name, cond_i));
        i
    }

    // ---- counter_loop -------------------------------------------------------

    pub fn make_flow_block_counter_loop(&mut self, name: &str, last_loop_cnt: i32) -> FlowBlockIdent {
        let i = self.add_flow_block_counter_loop(FlowBlockCounterLoop::new(name, last_loop_cnt));
        i
    }

    // ---- wait: SCWAIT -------------------------------------------------------

    // Stall the flow until `cond_i[cond_slice]` fires (WaitCondNode). The slice
    // is passed straight to the wait node — the cond-wait register slices the
    // condition natively, so no SliceBit expression is needed here. Defaults to
    // the whole variable when no slice is given.
    pub fn make_flow_block_scwait(&mut self, name: &str, cond_i: HcpIdent, cond_slice: Option<Slice>) -> FlowBlockIdent {
        let cond_sl = cond_slice.unwrap_or_else(|| Slice::new(0, self.get_hw_bit_sz(&cond_i)));
        let i = self.add_flow_block_wait(FlowBlockWait::new_cond_wait(name, cond_i, cond_sl));
        i
    }

    // ---- wait: SYWAIT -------------------------------------------------------

    // Stall the flow for a fixed `cycle` clock count (WaitCycleNode).
    pub fn make_flow_block_sywait(&mut self, name: &str, cycle: i32) -> FlowBlockIdent {
        let i = self.add_flow_block_wait(FlowBlockWait::new_cycle_wait(name, cycle));
        i
    }
}
