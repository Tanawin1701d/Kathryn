use crate::model::controller::clock_mode::{ClockMode, get_global_clk_mode};
use crate::model::hw_component::common::asm_mode::get_asm_pri_val;
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::hcp_accesible::HcpAccessible;
use crate::model::hw_component::common::hcp_assign::{HcpAssign, HcpAssignable};
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::hcp_read::HcpReadable;
use crate::model::hw_component::common::slice::Slice;
use crate::model::hw_component::common::update_event::UpdatingEvent;

pub struct Reg {
    assign: HcpAssign,
    ident    : HcpIdent,
    bit_width: i32,
}

impl HcpReadable for Reg {
    fn get_hcp_rdb_ident(&self) -> HcpIdent { self.ident.clone() }
}

impl HcpAssignable for Reg {
    fn get_hcp_assign    (&self)     -> &    HcpAssign { &self.assign }
    fn get_hcp_assign_mut(&mut self) -> &mut HcpAssign { &mut self.assign }

    fn get_hcp_asb_ident(&self) -> HcpIdent { self.ident.clone() }

    fn retrieve_clk_mode(&self) -> ClockMode { get_global_clk_mode() }

    fn get_des_slice(&self) -> Slice { Slice::new(0, self.bit_width) }

    fn get_priority(&self) -> i32 { get_asm_pri_val() }

    fn do_asm(&self,
              srci     : & HcpIdent,
              des_slice: & Option<Slice>,
              src_slice: & Slice,
              clk_mode : & Option<ClockMode>) -> AssignMeta {
        
        let mut asm = self.gen_asm_meta(&srci, des_slice, src_slice);
        if let Some(clk) = clk_mode {
            asm.get_input_event_mut().as_mut()
                      .unwrap().set_clk_mode(*clk);
        }
        asm
    }
}


impl HcpAccessible for Reg {
    fn get_bit_width(&self) -> usize { self.bit_width as usize }
}
