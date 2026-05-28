use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::asm_mode::get_asm_pri_val;
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::hcp_base::HcpBase;
use crate::model::hw_component::common::hcp_assign::{HcpAssign, HcpAssignable};
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HcpIdentifiable, HwComponentType};
use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::hw_component::common::slice::Slice;
use crate::model::model_arena::ModelArena;
use crate::util::math::vary_val::VaryVal;

#[derive(Default)]
pub struct Val {
    assign   : HcpAssign,
    ident    : HcpIdent,
    bit_width: i32,
    value    : VaryVal,
}

impl Val {
    pub fn new(is_user_com: bool, name: &str, bit_width: i32, init_val: u64) -> Self {
        Self {
            assign   : HcpAssign::new(),
            ident    : HcpIdent::new(HwComponentType::Val, is_user_com, name),
            bit_width,
            value    : VaryVal::from_u64(init_val, bit_width as usize),
        }
    }

    pub fn mk(name: &str, bit_width: i32) -> Self {
        Val::new(true, name, bit_width, 0)
    }

    pub fn get_ident    (&self)     -> HcpIdent       { self.ident }
    pub fn get_ident_mut(&mut self) -> &mut HcpIdent  { &mut self.ident }
    pub fn get_value    (&self)     -> &VaryVal        { &self.value }
}


impl HcpAssignable for Val {
    fn get_hcp_assign(&self) -> &HcpAssign { &self.assign }
    fn get_hcp_assign_mut(&mut self) -> &mut HcpAssign { &mut self.assign }

    fn retrieve_clk_mode(&self) -> ClockMode { ClockMode::ClkFree }

    fn get_des_slice(&self) -> Slice { Slice::new(0, self.bit_width) }

    fn get_priority(&self) -> i32 { get_asm_pri_val() }

    fn do_asm(&self,
              _srci: HcpIdent,
              _des_slice: Option<Slice>,
              _src_slice: Slice,
              _arena    : &mut ModelArena) -> AssignMeta {
        panic!("Val::do_asm should not be called")
    }

    fn gen_update_event(&self,
                        _srci: HcpIdent,
                        _des_slice: Option<Slice>,
                        _src_slice: Slice,
                        _arena    : &mut ModelArena,
    ) -> crate::model::hw_component::common::update_event_ident::UpdateEventIdent {
        panic!("Val::gen_update_event should not be called")
    }

    fn gen_asm_meta(&self,
                    _des_i    : HcpIdent,
                    _srci: HcpIdent,
                    _des_slice: Option<Slice>,
                    _src_slice: Slice,
                    _arena    : &mut ModelArena,
    ) -> AssignMeta {
        panic!("Val::gen_asm_meta should not be called")
    }

}
impl Identifiable for Val {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}

impl HcpIdentifiable for Val {
    fn get_ident_mut(&mut self) -> &mut HcpIdent { &mut self.ident }
}

impl HcpBase for Val {
    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_val(*self); }
}
