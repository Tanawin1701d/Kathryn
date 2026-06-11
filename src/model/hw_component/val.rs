use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::asm_mode::get_asm_pri_val;
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::hcp_base::HcpBase;
use crate::model::hw_component::common::hcp_assign::{HcpAssign, HcpAssignable};
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HcpIdentifiable, HcpSensitiveType, HwComponentType};
use crate::model::hw_component::common::update_event_ident::UpdateEventIdent;
use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::hw_component::common::slice::Slice;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::NcpIdent;
use crate::util::math::vary_val::VaryVal;

// Constant-value HCP; holds a fixed bit pattern and is never the destination of an assignment.
// All HcpAssignable methods panic — Val exists only as a source operand.
#[derive(Default)]
pub struct Val {
    assign   : HcpAssign,
    ident    : HcpIdent,
    bit_width: i32,
    value    : VaryVal, // the constant bit pattern
}

impl Val {
    // ---- constructors ----

    /// Full constructor; `init_val` is the constant bit pattern stored at creation.
    pub fn new(is_user_com: bool, name: &str, bit_width: i32, init_val: u64) -> Self {
        Self {
            assign   : HcpAssign::new(),
            ident    : HcpIdent::new(HwComponentType::Val, HcpSensitiveType::ReadOnly, is_user_com, name),
            bit_width,
            value    : VaryVal::from_u64(init_val, bit_width as usize),
        }
    }

    /// Variant that accepts an arbitrary-width `VaryVal` directly (for values > 64 bits).
    pub fn new_vv(is_user_com: bool, name: &str, bit_width: i32, value: VaryVal) -> Self {
        debug_assert_eq!(value.bit_width(), bit_width as usize, "VaryVal width mismatch");
        Self {
            assign   : HcpAssign::new(),
            ident    : HcpIdent::new(HwComponentType::Val, HcpSensitiveType::ReadOnly, is_user_com, name),
            bit_width,
            value,
        }
    }

    // ---- accessors ----

    pub fn get_ident    (&    self) -> HcpIdent       {      self.ident }
    pub fn get_ident_mut(&mut self) -> &mut HcpIdent  { &mut self.ident }
    pub fn get_value    (&    self) -> &VaryVal       { &    self.value }
}


// Val satisfies HcpAssignable only to fit the HCP trait hierarchy;
// it is never a write destination, so all mutating methods panic.
impl HcpAssignable for Val {
    fn get_hcp_assign    (&    self) -> &    HcpAssign { &self.assign }
    fn get_hcp_assign_mut(&mut self) -> &mut HcpAssign { &mut self.assign }

    fn retrieve_clk_mode(&self) -> ClockMode { ClockMode::ClkFree }

    fn get_des_slice(&self) -> Slice { Slice::new(0, self.bit_width) }

    fn get_priority(&self) -> i32 { get_asm_pri_val() }

    fn do_asm(&self,
              _srci     : HcpIdent,
              _des_slice: Option<Slice>,
              _src_slice: Slice,
              _arena    : &mut ModelArena) -> NcpIdent {
        panic!("Val::do_asm should not be called")
    }

    fn gen_update_event(&self,
                        _srci     : HcpIdent,
                        _des_slice: Option<Slice>,
                        _src_slice: Slice,
                        _arena    : &mut ModelArena,
    ) -> UpdateEventIdent {
        panic!("Val::gen_update_event should not be called")
    }

    fn gen_asm_meta(&self,
                    _srci     : HcpIdent,
                    _des_slice: Option<Slice>,
                    _src_slice: Slice,
                    _arena    : &mut ModelArena,
    ) -> AssignMeta {
        panic!("Val::gen_asm_meta should not be called")
    }

}

impl Identifiable for Val {
    fn get_ident_base    (&    self) -> &    IdentBase { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
}

impl HcpIdentifiable for Val {
    fn get_ident    (&    self) ->      HcpIdent { self.ident      }
    fn get_ident_mut(&mut self) -> &mut HcpIdent { &mut self.ident }
}

impl HcpBase for Val {
    // Each concrete type knows its own arena slot, so callers use zero match.
    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_val(*self); }
}
