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

/// n-bit synchronisation register: raised bit-by-bit as dependent states
/// activate; fully raised when all bits are 1.  Mirrors C++ `SyncReg`.
pub struct SyncReg {
    assign              : HcpAssign,
    ident               : HcpIdent,
    bit_width           : i32,
    next_fill_activate_id: i32,
    /// Populated by arena after construction.
    end_expr            : Option<HcpIdent>,
}

impl SyncReg {
    pub fn new(is_user_com: bool, name: &str, size: i32) -> Self {
        assert!(size > 0, "SyncReg size must be positive");
        Self {
            assign               : HcpAssign::new(),
            ident                : HcpIdent::new(HwComponentType::StateReg, is_user_com, name),
            bit_width            : size,
            next_fill_activate_id: 0,
            end_expr             : None,
        }
    }

    pub fn mk(name: &str, size: i32) -> Self { Self::new(false, name, size) }

    pub fn get_ident            (&self) -> HcpIdent { self.ident }
    pub fn get_next_activate_id (&self) -> i32      { self.next_fill_activate_id }

    pub fn set_end_expr(&mut self, expr: HcpIdent) { self.end_expr = Some(expr); }
}

impl HcpReadable for SyncReg {
    fn get_hcp_rdb_ident(&self) -> HcpIdent { self.ident }
}

impl HcpAssignable for SyncReg {
    fn get_hcp_assign    (&self)     -> &    HcpAssign { &self.assign }
    fn get_hcp_assign_mut(&mut self) -> &mut HcpAssign { &mut self.assign }

    fn get_hcp_asb_ident(&self) -> HcpIdent { self.ident }
    fn retrieve_clk_mode(&self) -> ClockMode { ClockMode::ClkFree }
    fn get_des_slice    (&self) -> Slice     { Slice::new(0, self.bit_width) }
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

impl HcpAccessible for SyncReg {
    fn get_bit_width(&self) -> usize { self.bit_width as usize }
}

impl Identifiable for SyncReg {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}

impl CtrlFlowRegBase for SyncReg {
    fn get_ctrl_ident(&self) -> HcpIdent { self.ident }

    fn add_depend_state(&mut self,
                        _depend_state  : HcpIdent,
                        _activate_cond : Option<HcpIdent>,
                        _cm            : ClockMode) {
        // Sets bit [next_fill_activate_id] to 1 when depend_state is active
        // and endExprInv holds; also updates testWire.
        // Increments next_fill_activate_id after each call.
        todo!("requires arena for expression/value/wire creation")
    }

    fn make_unset_state_event(&mut self, _cm: ClockMode) {
        // Clears all bits when (self | testWire) == upFullState
        todo!("requires arena for expression/value creation")
    }

    fn make_user_rst_event(&mut self, _rst: HcpIdent, _cm: ClockMode) {
        // Clears all bits unconditionally when rst fires
        todo!("requires arena for expression/value creation")
    }

    fn generate_end_expr(&self) -> HcpIdent {
        assert_eq!(self.next_fill_activate_id, self.bit_width,
            "not all SyncReg bits have been assigned a depend state");
        self.end_expr.expect("end_expr not set — call set_end_expr after arena setup")
    }
}
