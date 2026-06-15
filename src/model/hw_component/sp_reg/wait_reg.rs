use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::hcp_base::HcpBase;
use crate::model::hw_component::common::hcp_assign::{HcpAssign, HcpAssignable};
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HcpIdentifiable, HcpSensitiveType, HwComponentType};
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::hw_component::common::slice::Slice;
use crate::model::hw_component::common::update_event::{DEFAULT_UE_PRI_INTERNAL_MAX, DEFAULT_UE_PRI_INTERNAL_MIN, DEFAULT_UE_PRI_RST};
use crate::model::hw_component::common::util::check_ident_bit_size;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::NcpIdent;
use crate::model::hw_component::sp_reg::trigger_sig::{HasTriggerSig, TriggerSig};
use crate::model::common::identifier::{IdentBase, Identifiable};

const DEFAULT_UE_PRI_CW_UNSET : i32 = DEFAULT_UE_PRI_INTERNAL_MIN;
const DEFAULT_UE_PRI_CW_SET   : i32 = DEFAULT_UE_PRI_INTERNAL_MIN + 1;
const DEFAULT_UE_PRI_CW_HOLD  : i32 = DEFAULT_UE_PRI_INTERNAL_MIN + 2;
const DEFAULT_UE_PRI_CW_RST   : i32 = DEFAULT_UE_PRI_INTERNAL_MIN + 3;
const DEFAULT_UE_PRI_CW_MRST  : i32 = DEFAULT_UE_PRI_RST;

const DEFAULT_UE_PRI_CY_UNSET : i32 = DEFAULT_UE_PRI_INTERNAL_MIN;
const DEFAULT_UE_PRI_CY_INC   : i32 = DEFAULT_UE_PRI_INTERNAL_MIN + 1;
const DEFAULT_UE_PRI_CY_SET   : i32 = DEFAULT_UE_PRI_INTERNAL_MIN + 2;
const DEFAULT_UE_PRI_CY_HOLD  : i32 = DEFAULT_UE_PRI_INTERNAL_MIN + 3;
const DEFAULT_UE_PRI_CY_RST   : i32 = DEFAULT_UE_PRI_INTERNAL_MIN + 4;
const DEFAULT_UE_PRI_CY_MRST  : i32 = DEFAULT_UE_PRI_RST;

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
    assign       : HcpAssign,
    ident        : HcpIdent,
    triggers     : TriggerSig,
    cond_opr     : HcpIdent,
    cond_opr_sl  : Slice,
    up_state_i   : Option<HcpIdent>,
    down_state_i : Option<HcpIdent>,
    self_is_up_i : Option<HcpIdent>,
    end_expr_i   : Option<HcpIdent>,
}

impl Default for CondWaitStateReg {
    fn default() -> Self {
        Self {
            assign       : HcpAssign::new(),
            ident        : HcpIdent::default(),
            triggers     : TriggerSig::new(),
            cond_opr     : HcpIdent::default(),
            cond_opr_sl  : Slice::default(),
            up_state_i   : None,
            down_state_i : None,
            self_is_up_i : None,
            end_expr_i   : None,
        }
    }
}

impl CondWaitStateReg {
    pub fn new(is_user_com: bool, name: &str, cond_opr: HcpIdent, cond_opr_sl: Slice) -> Self {
        Self {
            assign       : HcpAssign::new(),
            ident        : HcpIdent::new(HwComponentType::CondWaitStateReg, HcpSensitiveType::Clocked, is_user_com, name),
            triggers     : TriggerSig::new(),
            cond_opr,
            cond_opr_sl,
            up_state_i   : None,
            down_state_i : None,
            self_is_up_i : None,
            end_expr_i   : None,
        }
    }

    pub fn get_ident      (&    self) -> HcpIdent         { self.ident      }
    pub fn get_ident_mut  (&mut self) -> &mut HcpIdent    { &mut self.ident }
    pub fn get_cond_opr   (&    self) -> HcpIdent         { self.cond_opr   }
    pub fn get_end_expr_i (&    self) -> Option<HcpIdent> { self.end_expr_i }

    /// Creates internal support constants/expressions used by update events.
    pub fn build_support_signal(&mut self, model_ar: &mut ModelArena) {
        let name = self.ident.get_ident_base().get_abs_name().to_string();

        let up_state_i = model_ar.make_val(false, &format!("{}_UP_STATE", name), 1, 1);
        self.up_state_i = Some(up_state_i);

        let down_state_i = model_ar.make_val(false, &format!("{}_DOWN_STATE", name), 1, 0);
        self.down_state_i = Some(down_state_i);

        // self == up_state — the "still waiting" guard reused by the unset UE.
        let self_is_up_i = model_ar.make_expression(
            false, &format!("{}_SELF_IS_UP", name), LogicOp::RelationEq, self.ident, up_state_i,
            Some(Slice::new(0, 1)), Some(Slice::new(0, 1)),
        );
        self.self_is_up_i = Some(self_is_up_i);

        let end_expr_i = model_ar.make_expression(
            false, &format!("{}_END_EXPR", name), LogicOp::BitwiseAnd, self.ident, self.cond_opr,
            Some(Slice::new(0, 1)), Some(self.cond_opr_sl),
        );
        self.end_expr_i = Some(end_expr_i);
    }

    /// Generates all update events.
    /// `build_support_signal` must be called first.
    pub fn build_update_event(&mut self, model_ar: &mut ModelArena) {
        let owner_name = self.get_global_name().to_string();
        let taken_sz = self.get_des_slice().get_size();
        self.triggers.integrity_check(&owner_name, model_ar, Some(self.ident), taken_sz);
        check_ident_bit_size(&self.cond_opr, 1, &owner_name, model_ar, Some(self.ident), taken_sz);

        let up_state_i = self.up_state_i.expect("build_support_signal must be called first");
        let down_state_i = self.down_state_i.expect("build_support_signal must be called first");
        let self_is_up_i = self.self_is_up_i.expect("build_support_signal must be called first");

        let bit_sl  = Slice::new(0, 1);
        let clk_src = self.get_clk_sig_i();

        // unset: createUE(cond_opr, self == upState, downState, {0,1}, MIN, cm)
        let ue_unset = model_ar.make_ue_full(
            Some(self.cond_opr)    , Some(self_is_up_i)      , down_state_i,
            bit_sl                 , bit_sl                  ,
            DEFAULT_UE_PRI_CW_UNSET, self.retrieve_clk_mode(), false, clk_src
        );
        self.add_update_event(ue_unset);

        // activate: createUE(activateCond, dependState, upState, {0,1}, MAX, cm)
        let nodes: Vec<_> = self.triggers.iter_depend_nodes().collect();
        for (srci, condi) in nodes {
            let ue = model_ar.make_ue_full(
                condi                , Some(srci)              , up_state_i,
                bit_sl               , bit_sl        ,
                DEFAULT_UE_PRI_CW_SET, self.retrieve_clk_mode(), false, clk_src
            );
            self.add_update_event(ue);
        }

        // user reset: createUE(None, rst, downState, {0,1}, MIN, cm)
        if let Some(rst_sig_i) = self.get_rst_sig_i() {
            let ue_rst = model_ar.make_ue_full(
                None                 , Some(rst_sig_i)         , down_state_i,
                bit_sl               , bit_sl                  ,
                DEFAULT_UE_PRI_CW_RST, self.retrieve_clk_mode(), false, clk_src
            );
            self.add_update_event(ue_rst);
        }

        // hold: rewrite self with self when hold_sig fires (suppresses lower-priority UEs)
        if let Some(hold_sig_i) = self.get_hold_sig_i() {
            let ue_hold = model_ar.make_ue_full(
                None                  , Some(hold_sig_i)        , self.ident,
                bit_sl                , bit_sl                  ,
                DEFAULT_UE_PRI_CW_HOLD, self.retrieve_clk_mode(), false, clk_src
            );
            self.add_update_event(ue_hold);
        }

        // master reset: createUE(None, mrst, downState, {0,1}, MRST, cm)
        if let Some(mrst_sig_i) = self.get_mrst_sig_i() {
            let ue_mrst = model_ar.make_ue_full(
                None                  , Some(mrst_sig_i)        , down_state_i,
                bit_sl                , bit_sl                  ,
                DEFAULT_UE_PRI_CW_MRST, self.retrieve_clk_mode(), false, clk_src
            );
            self.add_update_event(ue_mrst);
        }
    }

    pub fn generate_end_expr(&self) -> HcpIdent {
        self.end_expr_i.expect("build_support_signal must be called before generate_end_expr")
    }
}

impl HasTriggerSig for CondWaitStateReg {
    fn get_triggers    (&self)     -> &TriggerSig     { &self.triggers     }
    fn get_triggers_mut(&mut self) -> &mut TriggerSig { &mut self.triggers }
}

impl HcpAssignable for CondWaitStateReg {
    fn get_hcp_assign    (&self)     -> &    HcpAssign { &self.assign }
    fn get_hcp_assign_mut(&mut self) -> &mut HcpAssign { &mut self.assign }

    fn retrieve_clk_mode(&self) -> ClockMode { ClockMode::PosEdge }
    fn get_des_slice    (&self) -> Slice     { Slice::new(0, 1) }
    fn get_priority     (&self) -> i32       { DEFAULT_UE_PRI_INTERNAL_MIN }

    fn do_asm(&self,
              _srci: HcpIdent,
              _des_slice: Option<Slice>,
              _src_slice: Slice,
              _arena    : &mut ModelArena) -> NcpIdent {
        panic!("CondWaitStateReg::do_asm() is not supported; use build_update_event()")
    }
}

impl Identifiable for CondWaitStateReg {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
}

impl HcpIdentifiable for CondWaitStateReg {
    fn get_ident    (&    self) ->      HcpIdent { self.ident      }
    fn get_ident_mut(&mut self) -> &mut HcpIdent { &mut self.ident }
}

impl HcpBase for CondWaitStateReg {
    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_cond_wait_reg(*self); }
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
    assign          : HcpAssign,
    ident           : HcpIdent,
    triggers        : TriggerSig,
    wait_cycle      : Option<i32>,  // Some if constructed from a fixed cycle count
    cnt_bit_sz      : i32,
    total_bit_size  : i32,
    /// Handle to the `endCnt` operand.
    /// In `new_with_expr`, this is expected to be a full-width end marker.
    end_cnt         : HcpIdent,

    idle_cnt_i       : Option<HcpIdent>, // const 0: value when idle/inactive
    start_cnt_i      : Option<HcpIdent>, // const 0b11: initial value on activate (state bit set, counter=1)
    inc_step_i       : Option<HcpIdent>, // const 0b10: added each cycle to increment counter without touching bit 0
    end_full_i       : Option<HcpIdent>, // const (end_cnt<<1)|1: full-width terminal value
    is_active_expr_i : Option<HcpIdent>, // expr: self != idle_cnt
    is_end_expr_i    : Option<HcpIdent>, // expr: self == end_full
    is_not_end_expr_i: Option<HcpIdent>, // expr: self != end_full
    inc_cond_expr_i  : Option<HcpIdent>, // expr: is_active && is_not_end (increment gate)
    inc_expr_i       : Option<HcpIdent>, // expr: self + inc_step (next counter value)
}

impl Default for CycleWaitStateReg {
    fn default() -> Self {
        Self {
            assign          : HcpAssign::new(),
            ident           : HcpIdent::default(),
            triggers        : TriggerSig::new(),
            wait_cycle      : Some(1),
            cnt_bit_sz      : 1,
            total_bit_size  : 2,
            end_cnt         : HcpIdent::default(),
            idle_cnt_i      : None,
            start_cnt_i     : None,
            inc_step_i      : None,
            end_full_i      : None,
            is_active_expr_i: None,
            is_end_expr_i   : None,
            is_not_end_expr_i: None,
            inc_cond_expr_i : None,
            inc_expr_i      : None,
        }
    }
}

impl CycleWaitStateReg {
    /// Construct from a fixed wait-cycle count.
    pub fn new_with_cycle(is_user_com: bool, name: &str, wait_cycle: i32, end_cnt_ident: HcpIdent) -> Self {
        assert!(wait_cycle > 0, "wait_cycle must be positive");
        let cnt_bit_sz     = cal_bit_used(wait_cycle);
        let total_bit_size = cnt_bit_sz + 1;
        Self {
            assign          : HcpAssign::new(),
            ident           : HcpIdent::new(HwComponentType::CycleWaitStateReg, HcpSensitiveType::Clocked, is_user_com, name),
            triggers        : TriggerSig::new(),
            wait_cycle      : Some(wait_cycle),
            cnt_bit_sz,
            total_bit_size,
            end_cnt         : end_cnt_ident,
            idle_cnt_i      : None,
            start_cnt_i     : None,
            inc_step_i      : None,
            end_full_i      : None,
            is_active_expr_i: None,
            is_end_expr_i   : None,
            is_not_end_expr_i: None,
            inc_cond_expr_i : None,
            inc_expr_i      : None,
        }
    }

    /// Construct from an external end-count expression.
    pub fn new_with_expr(is_user_com: bool, name: &str, cnt_bit_sz: i32, end_cnt_ident: HcpIdent) -> Self {
        assert!(cnt_bit_sz > 0, "cnt_bit_sz must be positive");
        Self {
            assign          : HcpAssign::new(),
            ident           : HcpIdent::new(HwComponentType::CycleWaitStateReg, HcpSensitiveType::Clocked, is_user_com, name),
            triggers        : TriggerSig::new(),
            wait_cycle      : None,
            cnt_bit_sz,
            total_bit_size: cnt_bit_sz + 1,
            end_cnt         : end_cnt_ident,
            idle_cnt_i      : None,
            start_cnt_i     : None,
            inc_step_i      : None,
            end_full_i      : None,
            is_active_expr_i: None,
            is_end_expr_i   : None,
            is_not_end_expr_i: None,
            inc_cond_expr_i : None,
            inc_expr_i      : None,
        }
    }

    pub fn get_ident         (&self)     -> HcpIdent       { self.ident }
    pub fn get_ident_mut     (&mut self) -> &mut HcpIdent  { &mut self.ident }
    pub fn get_wait_cycle    (&self) -> Option<i32> { self.wait_cycle }
    pub fn get_cnt_bit_sz    (&self) -> i32         { self.cnt_bit_sz }
    pub fn get_total_bit_size(&self) -> i32         { self.total_bit_size }
    pub fn get_end_cnt       (&self) -> HcpIdent    { self.end_cnt }
    pub fn get_end_expr_i    (&self) -> Option<HcpIdent> { self.is_end_expr_i }

    /// Creates internal support constants/expressions used by update events.
    pub fn build_support_signal(&mut self, model_ar: &mut ModelArena) {
        let name = self.ident.get_ident_base().get_abs_name().to_string();

        let idle_cnt_i = model_ar.make_val(false, &format!("{}_IDLE_CNT", name), self.total_bit_size, 0);
        self.idle_cnt_i = Some(idle_cnt_i);

        // C++ startVal = 0b11 (state bit high, counter starts at 1)
        let start_cnt_i = model_ar.make_val(false, &format!("{}_START_CNT", name), self.total_bit_size, 0b11);
        self.start_cnt_i = Some(start_cnt_i);

        // full-width +2 keeps state bit untouched while incrementing counter bits.
        let inc_step_i = model_ar.make_val(false, &format!("{}_INC_STEP", name), self.total_bit_size, 0b10);
        self.inc_step_i = Some(inc_step_i);

        let end_full_i = if let Some(wait_cycle) = self.wait_cycle {
            let end_full = ((wait_cycle as u64) << 1) | 1;
            model_ar.make_val(false, &format!("{}_END_FULL", name), self.total_bit_size, end_full)
        } else {
            // external end marker should be full-width, e.g. (end_cnt << 1) | 1
            let end_sz = model_ar.get_hcp_assign(&self.end_cnt).get_des_slice().get_size();
            assert_eq!(
                end_sz, self.total_bit_size,
                "CycleWaitStateReg '{}': external end_cnt must be {}-bit full marker, got {}",
                name, self.total_bit_size, end_sz
            );
            self.end_cnt
        };
        self.end_full_i = Some(end_full_i);

        let self_sl = Slice::new(0, self.total_bit_size);

        let is_active_expr_i = model_ar.make_expression(
            false, &format!("{}_IS_ACTIVE", name), LogicOp::RelationNeq, self.ident, idle_cnt_i,
            Some(self_sl), Some(self_sl),
        );
        self.is_active_expr_i = Some(is_active_expr_i);

        let is_end_expr_i = model_ar.make_expression(
            false, &format!("{}_IS_END", name), LogicOp::RelationEq, self.ident, end_full_i,
            Some(self_sl), Some(self_sl),
        );
        self.is_end_expr_i = Some(is_end_expr_i);

        let is_not_end_expr_i = model_ar.make_expression(
            false, &format!("{}_IS_NOT_END", name), LogicOp::RelationNeq, self.ident, end_full_i,
            Some(self_sl), Some(self_sl),
        );
        self.is_not_end_expr_i = Some(is_not_end_expr_i);

        let inc_cond_expr_i = model_ar.make_expression(
            false, &format!("{}_INC_COND", name), LogicOp::BitwiseAnd, is_active_expr_i, is_not_end_expr_i,
            None, None,
        );
        self.inc_cond_expr_i = Some(inc_cond_expr_i);

        let inc_expr_i = model_ar.make_expression(
            false, &format!("{}_INC_EXPR", name), LogicOp::ArithPlus, self.ident, inc_step_i,
            Some(self_sl), Some(self_sl),
        );
        self.inc_expr_i = Some(inc_expr_i);
    }

    /// Generates all update events.
    /// `build_support_signal` must be called first.
    pub fn build_update_event(&mut self, model_ar: &mut ModelArena) {
        let cm = self.retrieve_clk_mode();
        let owner_name = self.get_global_name().to_string();
        self.triggers.integrity_check(&owner_name, model_ar, Some(self.ident), self.get_des_slice().get_size());

        let idle_cnt_i = self.idle_cnt_i.expect("build_support_signal must be called first");
        let start_cnt_i = self.start_cnt_i.expect("build_support_signal must be called first");
        let is_end_expr_i = self.is_end_expr_i.expect("build_support_signal must be called first");
        let inc_cond_expr_i = self.inc_cond_expr_i.expect("build_support_signal must be called first");
        let inc_expr_i = self.inc_expr_i.expect("build_support_signal must be called first");

        let full_sl = Slice::new(0, self.total_bit_size);
        let cnt_sl = Slice::new(1, self.total_bit_size);

        let clk_src = self.get_clk_sig_i();

        // unset: when end reached, reset to idle
        let ue_unset = model_ar.make_ue_full(
            Some(is_end_expr_i), None, idle_cnt_i,
            full_sl, full_sl,
            DEFAULT_UE_PRI_CY_UNSET, cm, false, clk_src
        );
        self.add_update_event(ue_unset);

        // increment: while active and not at end, increment counter
        let ue_inc = model_ar.make_ue_full(
            Some(inc_cond_expr_i), None, inc_expr_i,
            cnt_sl, cnt_sl,
            DEFAULT_UE_PRI_CY_INC, cm, false, clk_src
        );
        self.add_update_event(ue_inc);

        // set/activate: on trigger, load start_cnt
        let nodes: Vec<_> = self.triggers.iter_depend_nodes().collect();
        for (srci, condi) in nodes {
            let ue = model_ar.make_ue_full(
                condi, Some(srci), start_cnt_i,
                full_sl, full_sl,
                DEFAULT_UE_PRI_CY_SET, cm, false, clk_src
            );
            self.add_update_event(ue);
        }

        // hold: rewrite self with self when hold_sig fires (suppresses lower-priority UEs)
        if let Some(hold_sig_i) = self.get_hold_sig_i() {
            let ue_hold = model_ar.make_ue_full(
                None, Some(hold_sig_i), self.ident,
                full_sl, full_sl,
                DEFAULT_UE_PRI_CY_HOLD, cm, false, clk_src
            );
            self.add_update_event(ue_hold);
        }

        // user reset
        if let Some(rst_sig_i) = self.get_rst_sig_i() {
            let ue_rst = model_ar.make_ue_full(
                None, Some(rst_sig_i), idle_cnt_i,
                full_sl, full_sl,
                DEFAULT_UE_PRI_CY_RST, cm, false, clk_src
            );
            self.add_update_event(ue_rst);
        }

        // master reset
        if let Some(mrst_sig_i) = self.get_mrst_sig_i() {
            let ue_mrst = model_ar.make_ue_full(
                None, Some(mrst_sig_i), idle_cnt_i,
                full_sl, full_sl,
                DEFAULT_UE_PRI_CY_MRST, cm, false, clk_src
            );
            self.add_update_event(ue_mrst);
        }
    }

    pub fn generate_end_expr(&self) -> HcpIdent {
        self.is_end_expr_i.expect("build_support_signal must be called before generate_end_expr")
    }
}

impl HasTriggerSig for CycleWaitStateReg {
    fn get_triggers    (&self)     -> &TriggerSig     { &self.triggers     }
    fn get_triggers_mut(&mut self) -> &mut TriggerSig { &mut self.triggers }
}

impl HcpAssignable for CycleWaitStateReg {
    fn get_hcp_assign    (&self)     -> &    HcpAssign { &self.assign }
    fn get_hcp_assign_mut(&mut self) -> &mut HcpAssign { &mut self.assign }

    fn retrieve_clk_mode(&self) -> ClockMode { ClockMode::PosEdge }
    fn get_des_slice    (&self) -> Slice     { Slice::new(0, self.total_bit_size) }
    fn get_priority     (&self) -> i32       { DEFAULT_UE_PRI_INTERNAL_MIN }

    fn do_asm(&self,
              _srci: HcpIdent,
              _des_slice: Option<Slice>,
              _src_slice: Slice,
              _arena    : &mut ModelArena) -> NcpIdent {
        panic!("CycleWaitStateReg::do_asm() is not supported; use build_update_event()")
    }
}

impl Identifiable for CycleWaitStateReg {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
}

impl HcpIdentifiable for CycleWaitStateReg {
    fn get_ident    (&    self) ->      HcpIdent { self.ident      }
    fn get_ident_mut(&mut self) -> &mut HcpIdent { &mut self.ident }
}

impl HcpBase for CycleWaitStateReg {
    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_cycle_wait_reg(*self); }
}
