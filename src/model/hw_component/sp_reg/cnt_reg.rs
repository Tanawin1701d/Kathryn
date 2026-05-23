use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::hcp_base::HcpBase;
use crate::model::hw_component::common::hcp_assign::{HcpAssign, HcpAssignable};
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HcpIdentifiable, HwComponentType};
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::hw_component::common::slice::Slice;
use crate::model::hw_component::common::update_event::{DEFAULT_UE_PRI_INTERNAL_MIN, DEFAULT_UE_PRI_RST};
use crate::model::model_arena::ModelArena;
use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::hw_component::sp_reg::trigger_sig::{HasTriggerSig, TriggerSig};

const DEFAULT_UE_PRI_CNT_INC  : i32 = DEFAULT_UE_PRI_INTERNAL_MIN + 1;
const DEFAULT_UE_PRI_CNT_HOLD : i32 = DEFAULT_UE_PRI_INTERNAL_MIN + 2;
const DEFAULT_UE_PRI_CNT_RST  : i32 = DEFAULT_UE_PRI_INTERNAL_MIN + 3;
const DEFAULT_UE_PRI_CNT_INT  : i32 = DEFAULT_UE_PRI_INTERNAL_MIN + 4;
const DEFAULT_UE_PRI_CNT_MRST : i32 = DEFAULT_UE_PRI_RST;

/// Number of bits needed to count up to `max_number` (exclusive).
/// Mirrors C++ `calBitUsedInCounter`.
pub fn cal_bit_used_in_counter(max_number: i32) -> i32 {
    assert!(max_number > 0);
    (max_number as f64 * 2.0 - 1.0).log2() as i32
}

/// Free-running counter register.  Counts from 0; `generate_end_expr` is true
/// when the counter reaches `last_cycle - 1`.  Mirrors C++ `CounterReg`.
pub struct CntReg {
    assign     : HcpAssign,
    ident      : HcpIdent,
    // trigger signals
    triggers   : TriggerSig,
    cnt_bit_sz : i32,
    inc_val    : i32,
    last_cycle : i32,
    // val
    last_cycle_val: Option<HcpIdent>,
    zero_val      : Option<HcpIdent>,
    at_last_expr  : Option<HcpIdent>,
    inc_expr      : Option<HcpIdent>,



}

impl CntReg {
    pub fn new(is_user_com: bool,
               name       : &str,
               inc_val    : i32,
               last_cycle: i32) -> Self {
        assert!(last_cycle >= 0, "max_cycle must be positive");
        let cnt_bit_sz = cal_bit_used_in_counter(last_cycle);
        assert!(inc_val > 0, "inc_val must be positive");


        Self {
            assign         : HcpAssign::new(),
            ident          : HcpIdent::new(HwComponentType::CntReg, is_user_com, name),
            triggers       : TriggerSig::new(),
            cnt_bit_sz,
            inc_val,
            last_cycle,
            last_cycle_val: None,
            at_last_expr   : None,
            zero_val       : None,
            inc_expr       : None,
        }
    }

    pub fn mk(name: &str, max_cycle: i32) -> Self { Self::new(false, name, 1, max_cycle) }

    pub fn get_ident(&self) -> HcpIdent { self.ident }
    pub fn get_ident_mut(&mut self) -> &mut HcpIdent { &mut self.ident }
    pub fn get_loop_cnt  (&self) -> i32 { self.last_cycle }
    pub fn get_cnt_bit_sz(&self) -> i32 { self.cnt_bit_sz }
    pub fn generate_end_expr(&self) -> HcpIdent {
        self.at_last_expr.expect("build_support_signal must be called before generate_end_expr")
    }

    pub fn build_support_signal(&mut self, model_ar: &mut ModelArena) {
        
        let name = self.ident.get_ident_base().get_name().to_string();

        // constant: last_cycle - 1 (the maximum value the counter reaches before wrap)
        let last_cycle_val = model_ar.make_val(false, &format!("{}_LAST_CYCLE", name),
            self.cnt_bit_sz, (self.last_cycle - 1) as u64,
        );
        self.last_cycle_val = Some(last_cycle_val);

        // constant: 0 (the reset/wrap-back value)
        let zero_val = model_ar.make_val(false, &format!("{}_ZERO", name),
            self.cnt_bit_sz, 0,
        );
        self.zero_val = Some(zero_val);

        // expression: self == last_cycle_val  (1-bit; true when counter is at last cycle)
        let at_last_expr = model_ar.make_expression(false, &format!("{}_AT_LAST", name),
            LogicOp::RelationEq, self.ident, last_cycle_val, None, None);
        self.at_last_expr = Some(at_last_expr);

        // constant: inc_val (needed as the RHS of the add expression)
        let inc_val_val = model_ar.make_val(false, &format!("{}_INC_VAL", name),
            self.cnt_bit_sz, self.inc_val as u64,
        );

        // expression: self + inc_val  (next counter value)
        let inc_expr = model_ar.make_expression(false, &format!("{}_INC", name),
            LogicOp::ArithPlus, self.ident, inc_val_val, None, None,
        );
        self.inc_expr = Some(inc_expr);
    }

    pub fn build_update_event(&mut self, model_ar: &mut ModelArena) {



        let owner_name = self.build_unique_name().to_string();
        self.triggers.integrity_check(&owner_name, model_ar);

        let cnt_val_sl = Slice::new(0, self.cnt_bit_sz);

        let zero_val = self.zero_val.expect("build_support_signal must be called first");
        let inc_expr = self.inc_expr.expect("build_support_signal must be called first");


        // increment signal
        let nodes: Vec<_> = self.triggers.iter_depend_nodes().collect();
        for (srci, condi) in nodes {
            let priority = DEFAULT_UE_PRI_CNT_INC;
            let cm = self.retrieve_clk_mode();
            let ue = model_ar.make_ue_full(
                condi      , Some(srci), inc_expr,
                cnt_val_sl , cnt_val_sl ,
                priority   , cm        , false
            );
            self.add_update_event(ue);
        }

        // create the update event for the hold signal
        if let Some(hold_sig_i) = self.get_hold_sig_i() {
            let ue = model_ar.make_ue_full(
                None                   , Some(hold_sig_i)        , self.ident,
                cnt_val_sl             , cnt_val_sl              ,
                DEFAULT_UE_PRI_CNT_HOLD, self.retrieve_clk_mode(), false
            );
            self.add_update_event(ue);
        }

        // create the update event for the reset signal
        if let Some(rst_sig_i) = self.get_rst_sig_i() {
            let ue = model_ar.make_ue_full(
                None                  , Some(rst_sig_i)          , zero_val,
                cnt_val_sl            , cnt_val_sl                ,
                DEFAULT_UE_PRI_CNT_RST, self.retrieve_clk_mode() , false
            );
            self.add_update_event(ue);
        }

        // create the update event for the interrupt signal
        if let Some(int_sig_i) = self.get_int_sig_i() {
            let ue = model_ar.make_ue_full(
                None                  , Some(int_sig_i)          , inc_expr,
                cnt_val_sl            , cnt_val_sl                ,
                DEFAULT_UE_PRI_CNT_INT, self.retrieve_clk_mode() , false
            );
            self.add_update_event(ue);
        }

        // create the update event for the MASTER reset signal
        if let Some(mrst_sig_i) = self.get_mrst_sig_i() {
            let ue = model_ar.make_ue_full(
                None                   , Some(mrst_sig_i)         , zero_val,
                cnt_val_sl             , cnt_val_sl                ,
                DEFAULT_UE_PRI_CNT_MRST, self.retrieve_clk_mode() , false
            );
            self.add_update_event(ue);
        }
    }


}

impl Default for CntReg {
    fn default() -> Self {
        Self {
            assign         : HcpAssign::new(),
            ident          : HcpIdent::default(),
            triggers       : TriggerSig::new(),
            cnt_bit_sz     : 1,
            inc_val        : 1,
            last_cycle     : 1,
            last_cycle_val : None,
            zero_val       : None,
            at_last_expr   : None,
            inc_expr       : None,
        }
    }
}

impl HasTriggerSig for CntReg {
    fn get_triggers    (&self)     -> &TriggerSig     { &self.triggers     }
    fn get_triggers_mut(&mut self) -> &mut TriggerSig { &mut self.triggers }
}

impl HcpAssignable for CntReg {
    fn get_hcp_assign    (&self)     -> &    HcpAssign { &self.assign }
    fn get_hcp_assign_mut(&mut self) -> &mut HcpAssign { &mut self.assign }

    fn retrieve_clk_mode(&self) -> ClockMode { ClockMode::PosEdge }
    fn get_des_slice    (&self) -> Slice     { Slice::new(0, self.cnt_bit_sz) }
    fn get_priority     (&self) -> i32       { DEFAULT_UE_PRI_INTERNAL_MIN }

    fn do_asm(&self,
              srci     : &HcpIdent,
              des_slice: &Option<Slice>,
              src_slice: &Slice,
              arena    : &mut ModelArena) -> AssignMeta {
        self.gen_asm_meta(self.ident, srci, des_slice, src_slice, arena)
    }
}

impl Identifiable for CntReg {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}

impl HcpIdentifiable for CntReg {
    fn get_ident_mut(&mut self) -> &mut HcpIdent { &mut self.ident }
}

impl HcpBase for CntReg {
    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_cnt_reg(*self); }
}
