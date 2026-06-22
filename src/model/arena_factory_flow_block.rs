use crate::model::flow_block::{
    FlowBlockIdent,
    FlowBlockSeq, FlowBlockPar,
    FlowBlockCond, FlowBlockCondElif,
    FlowBlockZeroCondIf, FlowBlockZeroCondElif,
    FlowBlockZeroSwitch, FlowBlockZeroSwitchCase,
    FlowBlockPick, FlowBlockPickIf,
    FlowBlockWhile, FlowBlockDoWhile, FlowBlockCounterLoop,
    FlowBlockWait, FlowBlockPip, FlowBlockZync,
};
use crate::model::complex_hardware::arb::ArbLockedChannel;
use crate::model::complex_hardware::common::ccp_ident::CcpIdent;
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
    pub(crate) fn resolve_cond_slice(&mut self, name: &str, cond_i: HcpIdent, cond_slice: Option<Slice>) -> HcpIdent {
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

    // ---- pick: PICK ---------------------------------------------------------

    pub fn make_flow_block_pick(&mut self, name: &str) -> FlowBlockIdent {
        let i = self.add_flow_block_pick(FlowBlockPick::new(name));
        i
    }

    // ---- pick branch: PIF ---------------------------------------------------

    pub fn make_flow_block_pif(&mut self, name: &str, cond_i: HcpIdent, cond_slice: Option<Slice>) -> FlowBlockIdent {
        let cond_i = self.resolve_cond_slice(name, cond_i, cond_slice);
        let i = self.add_flow_block_pick_if(FlowBlockPickIf::new_pif(name, cond_i));
        i
    }

    // ---- pick branch: PIDEF -------------------------------------------------

    pub fn make_flow_block_pidef(&mut self, name: &str) -> FlowBlockIdent {
        let i = self.add_flow_block_pick_if(FlowBlockPickIf::new_pidef(name));
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

    // ---- pip: pipeline ------------------------------------------------------

    // Add the pip's leaf at `priority`: when `auto_req` it is Req-locked (always
    // requesting); otherwise it is a normal leaf.
    fn add_pip_leaf(&mut self, arb_i: CcpIdent, priority: i32, auto_req: bool) {
        if auto_req {
            self.arb_add_leaf_locked(arb_i, priority, ArbLockedChannel::Req);
        } else {
            self.arb_add_leaf(arb_i, priority);
        }
    }

    // Pipeline block gated by arbiter `arb_i`; holds exactly one body sub-block.
    // Adds the pip's leaf at `priority` (Req-locked when `auto_req`). When
    // `auto_restart` the arb user-reset re-launches the flow instead of clearing it.
    pub fn make_flow_block_pip(&mut self, name: &str, arb_i: CcpIdent, priority: i32, auto_req: bool, auto_restart: bool) -> FlowBlockIdent {
        self.add_pip_leaf(arb_i, priority, auto_req);
        self.add_flow_block_pip(FlowBlockPip::new(name, arb_i, auto_restart))
    }

    // ---- zync ---------------------------------------------------------------

    // Zync block contending on arbiter `arb_i`; its channel (leaf) is allocated
    // here at creation time with arbitration `priority`. When `auto_ack` the leaf
    // is Ack-locked (always granted); otherwise it is a normal contending leaf.
    pub fn make_flow_block_zync(&mut self, name: &str, arb_i: CcpIdent, priority: i32, auto_ack: bool) -> FlowBlockIdent {
        let channel_i = if auto_ack {
            self.arb_add_leaf_locked(arb_i, priority, ArbLockedChannel::Ack)
        } else {
            self.arb_add_leaf(arb_i, priority)
        };
        let block = FlowBlockZync::new(name, arb_i, channel_i);
        self.add_flow_block_zync(block)
    }
}
