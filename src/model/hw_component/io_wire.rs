use crate::model::controller::clock_mode::{ClockMode, get_global_clk_mode};
use crate::model::controller::asm_mode::get_asm_pri_val;
use crate::model::hw_component::common::hcp_base::HcpBase;
use crate::model::hw_component::common::hcp_assign::{HcpAssign, HcpAssignable};
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HcpIdentifiable, HcpSensitiveType, HwComponentType};
use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::hw_component::common::slice::Slice;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::NcpIdent;

#[derive(Default)]
pub struct IoWire {
    assign             : HcpAssign,
    ident              : HcpIdent,
    bit_width          : i32,
    is_input           : bool,
    // The logical signal this IoWire represents (used for reuse checks).
    actual_src_signal_i: Option<HcpIdent>,
    // The signal actually wired in bind_src (may differ, e.g. a boundary proxy).
    agent_src_signal_i : Option<HcpIdent>,
    // User-supplied port name overriding the auto-generated one (None = auto).
    explicit_name      : Option<String>,
}

impl IoWire {
    pub fn new(is_user_com      : bool,
               name             : &str,
               is_input         : bool,
               actual_src_i     : HcpIdent,
               agent_src_i      : HcpIdent,
               arena            : &mut ModelArena) -> Self {
        // bit_width is derived from the agent source; delegate the rest.
        let bit_width = arena.get_hw_bit_sz(&agent_src_i);
        Self::new_opt_src(is_user_com, name, is_input, bit_width,
                          Some(actual_src_i), Some(agent_src_i), arena)
    }

    // Like `new`, but the source signals are optional. `bit_width` must be given
    // explicitly since it cannot be derived without an agent source. When an
    // agent source is present its update event is bound; otherwise the wire is
    // left unbound (to be wired later).
    pub fn new_opt_src(is_user_com  : bool,
                       name         : &str,
                       is_input     : bool,
                       bit_width    : i32,
                       actual_src_i : Option<HcpIdent>,
                       agent_src_i  : Option<HcpIdent>,
                       arena        : &mut ModelArena) -> Self {

        let mut io_wire = IoWire {
            assign             : HcpAssign::new(),
            ident              : HcpIdent::new(HwComponentType::IoWire, HcpSensitiveType::Combinational, is_user_com, name),
            bit_width,
            is_input,
            actual_src_signal_i: actual_src_i,
            agent_src_signal_i : agent_src_i,
            explicit_name      : None,
        };

        // Bind only when an agent source exists.
        if let Some(agent_src_i) = agent_src_i {
            let src_slice = arena.get_hw_slice(&agent_src_i);
            io_wire.bind_src(agent_src_i, Some(src_slice), src_slice, None, None, None, arena);
        }

        io_wire
    }

    pub fn get_ident               (&self)     -> HcpIdent      { self.ident               }
    pub fn get_ident_mut           (&mut self) -> &mut HcpIdent { &mut self.ident          }
    pub fn get_is_input            (&self)     -> bool          { self.is_input            }
    pub fn get_actual_src_signal_i (&self)     -> HcpIdent      { self.actual_src_signal_i.expect("IoWire: actual_src_signal_i is None") }
    pub fn get_agent_src_signal_i  (&self)     -> HcpIdent      { self.agent_src_signal_i .expect("IoWire: agent_src_signal_i is None")  }
    pub fn get_explicit_name       (&self)     -> Option<&str>  { self.explicit_name.as_deref() }
    pub fn set_explicit_name       (&mut self, name: String)    { self.explicit_name = Some(name); }

    pub fn matches_signal(&self, actual_src_signal: HcpIdent, is_input: bool) -> bool {
        (self.is_input == is_input) && (self.get_actual_src_signal_i() == actual_src_signal)
    }
}



impl HcpAssignable for IoWire {
    fn get_hcp_assign    (&self)     -> &    HcpAssign { &self.assign }
    fn get_hcp_assign_mut(&mut self) -> &mut HcpAssign { &mut self.assign }

    fn retrieve_clk_mode(&self) -> ClockMode { ClockMode::ClkFree }

    fn get_des_slice(&self) -> Slice { Slice::new(0, self.bit_width) }

    fn get_priority(&self) -> i32 { get_asm_pri_val() }

    fn do_asm(&self,
              srci: HcpIdent,
              des_slice: Option<Slice>,
              src_slice: Slice,
              arena    : &mut ModelArena) -> NcpIdent {
        panic!("the io wire cannot be do_asm")
    }
}


impl Identifiable for IoWire {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
}

impl HcpIdentifiable for IoWire {
    fn get_ident    (&    self) ->      HcpIdent { self.ident      }
    fn get_ident_mut(&mut self) -> &mut HcpIdent { &mut self.ident }
}

impl HcpBase for IoWire {
    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_io_wire(*self); }
}
