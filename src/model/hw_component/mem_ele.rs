use crate::model::controller::clock_mode::{ClockMode, get_global_clk_mode};
use crate::model::hw_component::common::asm_mode::get_asm_pri_val;
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::hcp_base::HcpBase;
use crate::model::hw_component::common::hcp_assign::{HcpAssign, HcpAssignable};
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HcpIdentifiable, HwComponentType};
use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::hw_component::common::slice::Slice;
use crate::model::model_arena::ModelArena;

#[derive(Default)]
pub struct MemEle {
    assign           : HcpAssign,
    ident            : HcpIdent,
    master_mem_blk_i : HcpIdent, // owning MemBlk (name needed for Verilog emission)
    index_ident      : HcpIdent,
    bit_width        : i32,      // must match the owning MemBlk's bit_width
    is_read          : bool,
}

impl MemEle {
    // TODO : add to arena + add to IDEN and add to module Iden
    pub fn new(is_user_com      : bool,
               name             : &str,
               master_mem_blk_i : HcpIdent,
               index_ident      : HcpIdent,
               bit_width        : i32,
               is_read          : bool,
    ) -> Self {
        assert!(bit_width > 0, "bit_width must be positive, got {}", bit_width);
        Self {
            assign          : HcpAssign::new(),
            ident           : HcpIdent::new(HwComponentType::MemBlockIndexer, is_user_com, name),
            master_mem_blk_i,
            index_ident,
            bit_width,
            is_read,
        }
    }

    pub fn mk(name             : &str,
              master_mem_blk_i : HcpIdent,
              index_ident      : HcpIdent,
              bit_width        : i32,
              is_read          : bool,
    ) -> Self {
        MemEle::new(true, name, master_mem_blk_i, index_ident, bit_width, is_read)
    }

    pub fn get_ident            (&self)     -> HcpIdent       { self.ident            }
    pub fn get_ident_mut        (&mut self) -> &mut HcpIdent  { &mut self.ident       }
    pub fn get_master_mem_blk_i (&self)     -> HcpIdent       { self.master_mem_blk_i }
    pub fn get_index_ident      (&self)     -> HcpIdent       { self.index_ident      }
    pub fn is_read              (&self)     -> bool           { self.is_read          }
}

impl HcpAssignable for MemEle {
    fn get_hcp_assign    (&self)     -> &    HcpAssign { &self.assign }
    fn get_hcp_assign_mut(&mut self) -> &mut HcpAssign { &mut self.assign }

    fn retrieve_clk_mode(&self) -> ClockMode { get_global_clk_mode() }

    fn get_des_slice(&self) -> Slice { Slice::new(0, self.bit_width) }

    fn get_priority(&self) -> i32 { get_asm_pri_val() }

    fn do_asm(&self,
              srci: HcpIdent,
              des_slice: Option<Slice>,
              src_slice: Slice,
              arena    : &mut ModelArena) -> AssignMeta {
        self.gen_asm_meta(self.ident, srci, des_slice, src_slice, arena)
    }
}


impl Identifiable for MemEle {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}

impl HcpIdentifiable for MemEle {
    fn get_ident_mut(&mut self) -> &mut HcpIdent { &mut self.ident }
}

impl HcpBase for MemEle {
    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_mem_ele(*self); }
}
