use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::hcp_accesible::HcpAccessible;
use crate::model::hw_component::common::hcp_assign::{HcpAssign, HcpAssignable};
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HwComponentType};
use crate::model::hw_component::common::hcp_read::HcpReadable;
use crate::model::hw_component::common::slice::Slice;
use crate::model::hw_component::common::update_event::DEFAULT_UE_PRI_INTERNAL_MIN;
use crate::model::model_arena::ModelArena;
use crate::model::common::identifier::{IdentBase, Identifiable};

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
    cnt_bit_sz : i32,
    last_cycle : i32,
}

impl CntReg {
    pub fn new(is_user_com: bool, name: &str, max_cycle: i32) -> Self {
        assert!(max_cycle > 0, "max_cycle must be positive");
        let cnt_bit_sz = cal_bit_used_in_counter(max_cycle);
        Self {
            assign    : HcpAssign::new(),
            ident     : HcpIdent::new(HwComponentType::CntReg, is_user_com, name),
            cnt_bit_sz,
            last_cycle: max_cycle,
        }
    }

    pub fn mk(name: &str, max_cycle: i32) -> Self { Self::new(false, name, max_cycle) }

    pub fn get_ident     (&self) -> HcpIdent { self.ident }
    pub fn get_loop_cnt  (&self) -> i32      { self.last_cycle }
    pub fn get_cnt_bit_sz(&self) -> i32      { self.cnt_bit_sz }

    /// Build the increment event: counter += 1 whenever `up_count_event` fires.
    /// Mirrors C++ `makeIncEvent`.
    pub fn make_inc_event(&mut self, _up_count_event: Option<HcpIdent>, _cm: ClockMode) {
        // createUE(None, up_count_event, self + 1, {0, cnt_bit_sz}, MAX-1, cm)
        todo!("requires arena for expression creation")
    }
}

impl HcpReadable for CntReg {
    fn get_hcp_rdb_ident(&self) -> HcpIdent { self.ident }
}

impl HcpAssignable for CntReg {
    fn get_hcp_assign    (&self)     -> &    HcpAssign { &self.assign }
    fn get_hcp_assign_mut(&mut self) -> &mut HcpAssign { &mut self.assign }

    fn get_hcp_asb_ident(&self) -> HcpIdent { self.ident }
    fn retrieve_clk_mode(&self) -> ClockMode { ClockMode::ClkFree }
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

impl CtrlFlowRegBase for CntReg {
    fn get_ctrl_ident(&self) -> HcpIdent { self.ident }

    fn add_depend_state(&mut self,
                        _depend_state  : HcpIdent,
                        _activate_cond : Option<HcpIdent>,
                        _cm            : ClockMode) {
        // createUE(activate_cond, depend_state, idleVal, {0, cnt_bit_sz}, MAX, cm)
        todo!("requires arena for expression/value creation")
    }

    fn make_unset_state_event(&mut self, _cm: ClockMode) {
        panic!("CounterReg does not use makeUnSetStateEvent")
    }

    fn make_user_rst_event(&mut self, _rst: HcpIdent, _cm: ClockMode) {
        // createUE(None, rst, idleVal, {0, cnt_bit_sz}, MIN, cm)
        todo!("requires arena for expression/value creation")
    }

    fn generate_end_expr(&self) -> HcpIdent {
        // self == (last_cycle - 1)
        todo!("requires arena to build the equality expression")
    }
}
