use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::hcp_accesible::HcpAccessible;
use crate::model::hw_component::common::hcp_assign::{HcpAssign, HcpAssignable};
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HwComponentType};
use crate::model::hw_component::common::hcp_read::HcpReadable;
use crate::model::hw_component::common::slice::Slice;
use crate::model::hw_component::common::update_event::{DEFAULT_UE_PRI_INTERNAL_MIN, UpdatingEvent};
use crate::model::hw_component::sp_reg::ctrl_flow_reg_base::CtrlFlowRegBase;
use crate::model::common::identifier::{IdentBase, Identifiable};

// ---- helpers ----------------------------------------------------------------

/// Number of bits needed to represent values in `[0, max_number]`.
/// Mirrors C++ `calBitUsed`.
pub fn cal_bit_used(max_number: i32) -> i32 {
    assert!(max_number > 0);
    let amt = (max_number + 1) as f64;
    (amt * 2.0 - 1.0).log2() as i32
}

// ---- CondWaitStateReg -------------------------------------------------------

/// 1-bit conditional-wait state register: active until `cond_opr` fires.
/// Mirrors C++ `CondWaitStateReg`.
pub struct CondWaitStateReg {
    assign  : HcpAssign,
    ident   : HcpIdent,
    cond_opr: HcpIdent,
}

impl CondWaitStateReg {
    pub fn new(is_user_com: bool, name: &str, cond_opr: HcpIdent) -> Self {
        Self {
            assign  : HcpAssign::new(),
            ident   : HcpIdent::new(HwComponentType::CondWaitStateReg, is_user_com, name),
            cond_opr,
        }
    }

    pub fn mk(name: &str, cond_opr: HcpIdent) -> Self { Self::new(false, name, cond_opr) }

    pub fn get_ident   (&self) -> HcpIdent { self.ident }
    pub fn get_cond_opr(&self) -> HcpIdent { self.cond_opr }
}

impl HcpReadable for CondWaitStateReg {
    fn get_hcp_rdb_ident(&self) -> HcpIdent { self.ident }
}

impl HcpAssignable for CondWaitStateReg {
    fn get_hcp_assign    (&self)     -> &    HcpAssign { &self.assign }
    fn get_hcp_assign_mut(&mut self) -> &mut HcpAssign { &mut self.assign }

    fn get_hcp_asb_ident(&self) -> HcpIdent { self.ident }
    fn retrieve_clk_mode(&self) -> ClockMode { ClockMode::ClkFree }
    fn get_des_slice    (&self) -> Slice     { Slice::new(0, 1) }
    fn get_priority     (&self) -> i32       { DEFAULT_UE_PRI_INTERNAL_MIN }

    fn do_asm(&self,
              srci     : &HcpIdent,
              des_slice: &Option<Slice>,
              src_slice: &Slice,
              clk_mode : &Option<ClockMode>) -> AssignMeta {
        let mut asm = self.gen_asm_meta(srci, des_slice, src_slice);
        if let Some(clk) = clk_mode {
            asm.get_input_event_mut().as_mut().unwrap().set_clk_mode(*clk);
        }
        asm
    }
}

impl HcpAccessible for CondWaitStateReg {
    fn get_bit_width(&self) -> usize { 1 }
}

impl Identifiable for CondWaitStateReg {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}

impl CtrlFlowRegBase for CondWaitStateReg {
    fn get_ctrl_ident(&self) -> HcpIdent { self.ident }

    fn add_depend_state(&mut self,
                        _depend_state  : HcpIdent,
                        _activate_cond : Option<HcpIdent>,
                        _cm            : ClockMode) {
        // createUE(activate_cond, depend_state, upState, {0,1}, MAX, cm)
        todo!("requires arena for expression/value creation")
    }

    fn make_unset_state_event(&mut self, _cm: ClockMode) {
        // createUE(cond_opr, self == upState, downState, {0,1}, MIN, cm)
        todo!("requires arena for expression/value creation")
    }

    fn make_user_rst_event(&mut self, _rst: HcpIdent, _cm: ClockMode) {
        // createUE(None, rst, downState, {0,1}, MIN, cm)
        todo!("requires arena for expression/value creation")
    }

    fn generate_end_expr(&self) -> HcpIdent {
        // cond_opr & (self == upState)
        todo!("requires arena to build the AND expression")
    }
}

// ---- CycleWaitStateReg ------------------------------------------------------

/// Cycle-count wait register: holds a state bit (bit 0) and a counter
/// (bits 1..total).  Active until the counter reaches `end_cnt`.
///
/// Layout:   [ cnt_bits | state_bit ]
///                         ^--- bit 0
///
/// Mirrors C++ `CycleWaitStateReg`.
pub struct CycleWaitStateReg {
    assign        : HcpAssign,
    ident         : HcpIdent,
    wait_cycle    : Option<i32>,  // Some if constructed from a fixed cycle count
    cnt_bit_sz    : i32,
    total_bit_size: i32,
    /// Handle to the `endCnt` operand (fixed value or external expression).
    end_cnt       : HcpIdent,
}

impl CycleWaitStateReg {
    /// Construct from a fixed wait-cycle count.
    pub fn new_with_cycle(is_user_com: bool, name: &str, wait_cycle: i32, end_cnt_ident: HcpIdent) -> Self {
        assert!(wait_cycle > 0, "wait_cycle must be positive");
        let cnt_bit_sz     = cal_bit_used(wait_cycle);
        let total_bit_size = cnt_bit_sz + 1;
        Self {
            assign        : HcpAssign::new(),
            ident         : HcpIdent::new(HwComponentType::CycleWaitStateReg, is_user_com, name),
            wait_cycle    : Some(wait_cycle),
            cnt_bit_sz,
            total_bit_size,
            end_cnt       : end_cnt_ident,
        }
    }

    /// Construct from an external end-count expression.
    pub fn new_with_expr(is_user_com: bool, name: &str, cnt_bit_sz: i32, end_cnt_ident: HcpIdent) -> Self {
        assert!(cnt_bit_sz > 0, "cnt_bit_sz must be positive");
        Self {
            assign        : HcpAssign::new(),
            ident         : HcpIdent::new(HwComponentType::CycleWaitStateReg, is_user_com, name),
            wait_cycle    : None,
            cnt_bit_sz,
            total_bit_size: cnt_bit_sz + 1,
            end_cnt       : end_cnt_ident,
        }
    }

    pub fn get_ident         (&self) -> HcpIdent    { self.ident }
    pub fn get_wait_cycle    (&self) -> Option<i32> { self.wait_cycle }
    pub fn get_cnt_bit_sz    (&self) -> i32         { self.cnt_bit_sz }
    pub fn get_total_bit_size(&self) -> i32         { self.total_bit_size }
    pub fn get_end_cnt       (&self) -> HcpIdent    { self.end_cnt }

    /// Build the increment event: counter += 1 while state is active and
    /// counter has not yet reached end_cnt.  Mirrors C++ `makeIncStateEvent`.
    pub fn make_inc_state_event(&mut self, _hold_signal: Option<HcpIdent>, _cm: ClockMode) {
        // incCond = (self[1..total] != end_cnt) && hold_signal?
        // createUE(incCond, self[0], self[1..total] + 1, {1, total}, MAX-1, cm)
        todo!("requires arena for expression creation")
    }
}

impl HcpReadable for CycleWaitStateReg {
    fn get_hcp_rdb_ident(&self) -> HcpIdent { self.ident }
}

impl HcpAssignable for CycleWaitStateReg {
    fn get_hcp_assign    (&self)     -> &    HcpAssign { &self.assign }
    fn get_hcp_assign_mut(&mut self) -> &mut HcpAssign { &mut self.assign }

    fn get_hcp_asb_ident(&self) -> HcpIdent { self.ident }
    fn retrieve_clk_mode(&self) -> ClockMode { ClockMode::ClkFree }
    fn get_des_slice    (&self) -> Slice     { Slice::new(0, self.total_bit_size) }
    fn get_priority     (&self) -> i32       { DEFAULT_UE_PRI_INTERNAL_MIN }

    fn do_asm(&self,
              srci     : &HcpIdent,
              des_slice: &Option<Slice>,
              src_slice: &Slice,
              clk_mode : &Option<ClockMode>) -> AssignMeta {
        let mut asm = self.gen_asm_meta(srci, des_slice, src_slice);
        if let Some(clk) = clk_mode {
            asm.get_input_event_mut().as_mut().unwrap().set_clk_mode(*clk);
        }
        asm
    }
}

impl HcpAccessible for CycleWaitStateReg {
    fn get_bit_width(&self) -> usize { self.total_bit_size as usize }
}

impl Identifiable for CycleWaitStateReg {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}

impl CtrlFlowRegBase for CycleWaitStateReg {
    fn get_ctrl_ident(&self) -> HcpIdent { self.ident }

    fn add_depend_state(&mut self,
                        _depend_state  : HcpIdent,
                        _activate_cond : Option<HcpIdent>,
                        _cm            : ClockMode) {
        // createUE(activate_cond, depend_state, startCnt, {0, total}, MAX, cm)
        todo!("requires arena for expression/value creation")
    }

    fn make_unset_state_event(&mut self, _cm: ClockMode) {
        // createUE(self[1..total] == end_cnt, self[0], idleVal, {0, total}, MIN, cm)
        todo!("requires arena for expression/value creation")
    }

    fn make_user_rst_event(&mut self, _rst: HcpIdent, _cm: ClockMode) {
        // createUE(None, rst, idleVal, {0, total}, MIN, cm)
        todo!("requires arena for expression/value creation")
    }

    fn generate_end_expr(&self) -> HcpIdent {
        // self[0] & (self[1..total] == end_cnt)
        todo!("requires arena to build the AND expression")
    }
}
