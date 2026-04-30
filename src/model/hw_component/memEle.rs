use crate::model::controller::clock_mode::{ClockMode, get_global_clk_mode};
use crate::model::hw_component::common::asm_mode::get_asm_pri_val;
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::hcp_accesible::HcpAccessible;
use crate::model::hw_component::common::hcp_assign::{HcpAssign, HcpAssignable};
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HwComponentType};
use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::hw_component::common::hcp_read::HcpReadable;
use crate::model::hw_component::common::slice::Slice;
use crate::model::model_arena::ModelArena;

pub struct MemEle {
    assign      : HcpAssign,
    ident       : HcpIdent,
    index_ident : HcpIdent,
    bit_width   : i32, // bit width should match the mem block
    is_read     : bool
}

impl MemEle {
    // TODO : add to arena + add to IDEN and add to module Iden
    pub fn new(is_user_com: bool,
               name       : &str,
               index_ident: HcpIdent,
               bit_width  : i32,
               is_read    : bool) -> Self {
        assert!(bit_width > 0, "bit_width must be positive, got {}", bit_width);
        Self {
            assign      : HcpAssign::new(),
            ident       : HcpIdent::new(HwComponentType::MemBlockIndexer, is_user_com, name),
            index_ident,
            bit_width,
            is_read,
        }
    }

    pub fn mk(name       : &str,
              index_ident: HcpIdent,
              bit_width  : i32,
              is_read    : bool) -> Self {
        MemEle::new(true, name, index_ident, bit_width, is_read)
    }

    pub fn get_ident(&self) -> HcpIdent { self.ident }
}

impl HcpReadable for MemEle {
    fn get_hcp_rdb_ident(&self) -> HcpIdent { self.ident }
}

impl HcpAssignable for MemEle {
    fn get_hcp_assign    (&self)     -> &    HcpAssign { &self.assign }
    fn get_hcp_assign_mut(&mut self) -> &mut HcpAssign { &mut self.assign }

    fn get_hcp_asb_ident(&self) -> HcpIdent { self.ident }

    fn retrieve_clk_mode(&self) -> ClockMode { get_global_clk_mode() }

    fn get_des_slice(&self) -> Slice { Slice::new(0, self.bit_width) }

    fn get_priority(&self) -> i32 { get_asm_pri_val() }

    fn do_asm(&self,
              srci     : &HcpIdent,
              des_slice: &Option<Slice>,
              src_slice: &Slice,
              arena    : &mut ModelArena) -> AssignMeta {
        self.gen_asm_meta(srci, des_slice, src_slice, arena)
    }
}


impl HcpAccessible for MemEle {
    fn get_bit_width(&self) -> usize { self.bit_width as usize }
}

impl Identifiable for MemEle {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}