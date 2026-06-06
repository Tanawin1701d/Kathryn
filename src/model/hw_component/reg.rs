use crate::model::controller::clock_mode::{ClockMode, get_global_clk_mode};
use crate::model::hw_component::common::asm_mode::get_asm_pri_val;
use crate::model::hw_component::common::hcp_base::HcpBase;
use crate::model::hw_component::common::hcp_assign::{HcpAssign, HcpAssignable};
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HcpIdentifiable, HcpSensitiveType, HwComponentType};
use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::hw_component::common::slice::Slice;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::NcpIdent;

// General-purpose clocked register; the most common HCP type.
#[derive(Default)]
pub struct Reg {
    assign    : HcpAssign,
    ident     : HcpIdent,
    bit_width : i32,
}

impl Reg {
    // ---- constructors ----

    /// Full constructor; `is_user_com` false for system-generated regs.
    pub fn new(is_user_com: bool, name: &str, bit_width: i32) -> Self {
        Self {
            assign   : HcpAssign::new(),
            ident    : HcpIdent::new(HwComponentType::Reg, HcpSensitiveType::Clocked, is_user_com, name),
            bit_width,
        }
    }

    /// Shorthand for user-declared regs (`is_user_com = true`).
    pub fn mk(name: &str, bit_width: i32) -> Self {
        Reg::new(true, name, bit_width)
    }

    // ---- accessors ----

    pub fn get_ident(&self) -> HcpIdent { self.ident }
    pub fn get_ident_mut(&mut self) -> &mut HcpIdent { &mut self.ident }
}



impl HcpAssignable for Reg {
    fn get_hcp_assign    (&self)     -> &    HcpAssign { &self.assign }
    fn get_hcp_assign_mut(&mut self) -> &mut HcpAssign { &mut self.assign }

    fn retrieve_clk_mode(&self) -> ClockMode { get_global_clk_mode() }

    fn get_des_slice(&self) -> Slice { Slice::new(0, self.bit_width) }

    fn get_priority(&self) -> i32 { get_asm_pri_val() }

    fn do_asm(&self,
              srci       : HcpIdent,
              des_slice  : Option<Slice>,
              src_slice  : Slice,
              arena      : &mut ModelArena,
    ) -> NcpIdent {

        self.gen_asm_node(srci, des_slice, src_slice, arena)
    }
}


impl Identifiable for Reg {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}

impl HcpIdentifiable for Reg {
    fn get_ident    (&    self) ->      HcpIdent { self.ident      }
    fn get_ident_mut(&mut self) -> &mut HcpIdent { &mut self.ident }
}

impl HcpBase for Reg {
    // Each concrete type knows its own arena slot, so callers use zero match.
    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_reg(*self); }
}
