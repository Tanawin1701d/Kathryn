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
pub struct IoWire {
    assign             : HcpAssign,
    ident              : HcpIdent,
    bit_width          : i32,
    is_input           : bool,
    // The logical signal this IoWire represents (used for reuse checks).
    actual_src_signal_i: HcpIdent,
    // The signal actually wired in bind_src (may differ, e.g. a boundary proxy).
    agent_src_signal_i : HcpIdent,
}

impl IoWire {
    pub fn new(is_user_com      : bool,
               name             : &str,
               is_input         : bool,
               actual_src_i     : HcpIdent,
               agent_src_i      : HcpIdent,
               arena            : &mut ModelArena) -> Self {

        let bit_width = arena.get_hw_bit_sz(&agent_src_i);
        let src_slice = arena.get_hw_slice(&agent_src_i);
        let des_slice = Some(src_slice);

        let mut io_wire = IoWire {
            assign             : HcpAssign::new(),
            ident              : HcpIdent::new(HwComponentType::IoWire, is_user_com, name),
            bit_width,
            is_input,
            actual_src_signal_i: actual_src_i,
            agent_src_signal_i : agent_src_i,
        };

        io_wire.bind_src(&agent_src_i, &des_slice, &src_slice, arena);

        io_wire
    }

    pub fn get_ident               (&self)     -> HcpIdent      { self.ident }
    pub fn get_ident_mut           (&mut self) -> &mut HcpIdent { &mut self.ident }
    pub fn get_is_input            (&self)     -> bool           { self.is_input }
    pub fn get_actual_src_signal_i (&self)     -> HcpIdent      { self.actual_src_signal_i }
    pub fn get_agent_src_signal_i  (&self)     -> HcpIdent      { self.agent_src_signal_i }

    pub fn matches_signal(&self, actual_src_signal: HcpIdent, is_input: bool) -> bool {
        self.is_input == is_input && self.actual_src_signal_i == actual_src_signal
    }

    pub fn bind_src(&mut self, src_i: &HcpIdent, des_slice: &Option<Slice>, src_slice: &Slice, arena: &mut ModelArena) {
        let ue = self.gen_update_event(src_i, des_slice, src_slice, arena);
        self.add_update_event(ue);
    }
}



impl HcpAssignable for IoWire {
    fn get_hcp_assign    (&self)     -> &    HcpAssign { &self.assign }
    fn get_hcp_assign_mut(&mut self) -> &mut HcpAssign { &mut self.assign }

    fn retrieve_clk_mode(&self) -> ClockMode { ClockMode::ClkFree }

    fn get_des_slice(&self) -> Slice { Slice::new(0, self.bit_width) }

    fn get_priority(&self) -> i32 { get_asm_pri_val() }

    fn do_asm(&self,
              srci     : &HcpIdent,
              des_slice: &Option<Slice>,
              src_slice: &Slice,
              arena    : &mut ModelArena) -> AssignMeta {
        panic!("the io wire cannot be do_asm")
    }
}


impl Identifiable for IoWire {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}

impl HcpIdentifiable for IoWire {
    fn get_ident_mut(&mut self) -> &mut HcpIdent { &mut self.ident }
}

impl HcpBase for IoWire {
    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_io_wire(*self); }
}
