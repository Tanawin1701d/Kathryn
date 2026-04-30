use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::hcp_accesible::HcpAccessible;
use crate::model::hw_component::common::hcp_assign::{HcpAssign, HcpAssignable};
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HwComponentType};
use crate::model::hw_component::common::hcp_read::HcpReadable;
use crate::model::hw_component::common::operation::{LogicOp, LOGICAL_SIZE};
use crate::model::hw_component::common::slice::Slice;
use crate::model::hw_component::common::update_event::DEFAULT_UE_PRI_INTERNAL_MIN;
use crate::model::model_arena::ModelArena;
use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::hw_component::sp_reg::trigger_sig::TriggerSig;

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
    last_Cycle_val: Option<HcpIdent>,
    at_last_expr: Option<HcpIdent>,
    zero_val    : Option<HcpIdent>,
    inc_expr    : Option<HcpIdent>,



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
            last_Cycle_val : None,
            at_last_expr   : None,
            zero_val       : None,
            inc_expr       : None,
        }
    }

    pub fn mk(name: &str, max_cycle: i32) -> Self { Self::new(false, name, 1, max_cycle) }

    pub fn build_support_signal(&mut self, model_ar: &mut ModelArena) {
        let name = self.ident.get_ident_base().get_name().to_string();

        // constant: last_cycle - 1 (the maximum value the counter reaches before wrap)
        let last_cycle_val = model_ar.make_val(&format!("{}_LAST_CYCLE", name),
            self.cnt_bit_sz, (self.last_cycle - 1) as u64,
        );
        self.last_Cycle_val = Some(last_cycle_val);

        // constant: 0 (the reset/wrap-back value)
        let zero_val = model_ar.make_val(&format!("{}_ZERO", name),
            self.cnt_bit_sz, 0,
        );
        self.zero_val = Some(zero_val);

        // expression: self == last_cycle_val  (1-bit; true when counter is at last cycle)
        let at_last_expr = model_ar.make_expression(&format!("{}_AT_LAST", name),
            LogicOp::RelationEq, self.ident, last_cycle_val);
        self.at_last_expr = Some(at_last_expr);

        // constant: inc_val (needed as the RHS of the add expression)
        let inc_val_val = model_ar.make_val(&format!("{}_INC_VAL", name),
            self.cnt_bit_sz, self.inc_val as u64,
        );

        // expression: self + inc_val  (next counter value)
        let inc_expr = model_ar.make_expression(&format!("{}_INC", name),
            LogicOp::ArithPlus, self.ident, inc_val_val
        );
        self.inc_expr = Some(inc_expr);
    }

    pub fn build_update_event(&mut self, model_ar: &mut ModelArena) {



    }


}

impl HcpReadable for CntReg {
    fn get_hcp_rdb_ident(&self) -> HcpIdent { self.ident }
}

impl HcpAssignable for CntReg {
    fn get_hcp_assign    (&self)     -> &    HcpAssign { &self.assign }
    fn get_hcp_assign_mut(&mut self) -> &mut HcpAssign { &mut self.assign }

    fn get_hcp_asb_ident(&self) -> HcpIdent { self.ident }
    fn retrieve_clk_mode(&self) -> ClockMode { ClockMode::PosEdge }
    fn get_des_slice    (&self) -> Slice     { Slice::new(0, self.cnt_bit_sz) }
    fn get_priority     (&self) -> i32       { DEFAULT_UE_PRI_INTERNAL_MIN }

    fn do_asm(&self,
              srci     : &HcpIdent,
              des_slice: &Option<Slice>,
              src_slice: &Slice,
              arena    : &mut ModelArena) -> AssignMeta {
        self.gen_asm_meta(srci, des_slice, src_slice, arena)
    }
}

impl HcpAccessible for CntReg {
    fn get_bit_width(&self) -> usize { self.cnt_bit_sz as usize }
}

impl Identifiable for CntReg {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}
