use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::asm_mode::get_asm_pri_val;
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::hcp_accesible::HcpAccessible;
use crate::model::hw_component::common::hcp_assign::{HcpAssign, HcpAssignable};
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HwComponentType};
use crate::model::hw_component::common::hcp_read::HcpReadable;
use crate::model::hw_component::common::slice::Slice;
use crate::model::hw_component::common::update_event::{UeBasic, UpdatingEvent};
use crate::util::math::vary_val::VaryVal;

pub struct Val {
    assign   : HcpAssign,
    ident    : HcpIdent,
    bit_width: i32,
    value    : VaryVal,
}

impl Val {
    pub fn new(is_user_com: bool, name: &str, bit_width: i32) -> Self {
        Self {
            assign   : HcpAssign::new(),
            ident    : HcpIdent::new(HwComponentType::Val, is_user_com, name),
            bit_width,
            value    : VaryVal::new(bit_width as usize),
        }
    }

    pub fn mk(name: &str, bit_width: i32) -> Self {
        Val::new(true, name, bit_width)
    }
}


impl HcpReadable for Val {
    fn get_hcp_rdb_ident(&self) -> HcpIdent { self.ident.clone() }
}

impl HcpAssignable for Val {
    fn get_hcp_assign(&self) -> &HcpAssign { &self.assign }
    fn get_hcp_assign_mut(&mut self) -> &mut HcpAssign { &mut self.assign }

    fn get_hcp_asb_ident(&self) -> HcpIdent { self.ident.clone() }

    fn retrieve_clk_mode(&self) -> ClockMode { ClockMode::ClkFree }

    fn get_des_slice(&self) -> Slice { Slice::new(0, self.bit_width) }

    fn get_priority(&self) -> i32 { get_asm_pri_val() }

    fn do_asm(&self,
              srci: &HcpIdent,
              des_slice: &Option<Slice>,
              src_slice: &Slice,
              clk_mode: &Option<ClockMode>) -> AssignMeta {
        panic!("Val::do_asm should not be called")
    }

    fn gen_update_event(&self,
                        srci: &HcpIdent,
                        des_slice: &Option<Slice>,
                        src_slice: &Slice,
    ) -> UeBasic {
        panic!("Val::gen_update_event should not be called")
    }

    fn gen_asm_meta(&self,
                    srci: &HcpIdent,
                    des_slice: &Option<Slice>,
                    src_slice: &Slice,
    ) -> AssignMeta {
        panic!("Val::gen_asm_meta should not be called")
    }


}
impl HcpAccessible for Val {
    fn get_bit_width(&self) -> usize { self.bit_width as usize }
}
