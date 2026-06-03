use crate::model::controller::clock_mode::{ClockMode, get_global_clk_mode};
use crate::model::hw_component::common::asm_mode::get_asm_pri_val;
use crate::model::hw_component::common::hcp_base::HcpBase;
use crate::model::hw_component::common::hcp_assign::{HcpAssign, HcpAssignable};
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HcpIdentifiable, HwComponentType};
use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::hw_component::common::slice::Slice;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::NcpIdent;

// Combinational wire; always ClkFree, no state.
#[derive(Default)]
pub struct Wire {
    assign   : HcpAssign,  // update-event pool and assignable dispatch
    ident    : HcpIdent,   // arena handle + HW type tag
    bit_width: i32,        // signal width in bits
}

impl Wire {
    // ---- constructors ----

    /// Full constructor; `is_user_com` false for system-generated wires.
    pub fn new(is_user_com: bool, name: &str, bit_width: i32) -> Self {
        Self {
            assign   : HcpAssign::new(),
            ident    : HcpIdent::new(HwComponentType::Wire, is_user_com, name),
            bit_width,
        }
    }

    // ---- accessors ----

    pub fn get_ident    (&    self) ->      HcpIdent { self.ident }
    pub fn get_ident_mut(&mut self) -> &mut HcpIdent { &mut self.ident }
}



impl HcpAssignable for Wire {
    // ---- pool accessors ----

    fn get_hcp_assign    (&self)     -> &    HcpAssign { &self.assign }
    fn get_hcp_assign_mut(&mut self) -> &mut HcpAssign { &mut self.assign }

    // ---- Wire-specific overrides ----

    // Wires are always combinational; the Verilog backend relies on ClkFree to
    // emit a continuous assign rather than a clocked always-block.
    fn retrieve_clk_mode(&self) -> ClockMode { ClockMode::ClkFree }

    // Full-width destination slice — wires have no sub-field partial writes.
    fn get_des_slice(&self) -> Slice { Slice::new(0, self.bit_width) }

    fn get_priority(&self) -> i32 { get_asm_pri_val() }

    // Delegates to the shared helper; self.ident is passed as the destination handle.
    fn do_asm(&self,
              srci     : HcpIdent,
              des_slice: Option<Slice>,
              src_slice: Slice,
              arena    : &mut ModelArena) -> NcpIdent {
        self.gen_asm_node(self.ident, srci, des_slice, src_slice, arena)
    }
}


// All three delegate to the inner HcpIdent, which forwards to its IdentBase.
impl Identifiable for Wire {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}

impl HcpIdentifiable for Wire {
    fn get_ident_mut(&mut self) -> &mut HcpIdent { &mut self.ident }
}

impl HcpBase for Wire {
    // Each concrete type knows its own arena slot, so callers use zero match.
    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_wire(*self); }
}
