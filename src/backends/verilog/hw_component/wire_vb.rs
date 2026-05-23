use crate::backends::verilog::hw_component::common::hcp_base_vb::HcpBaseVb;
use crate::backends::verilog::hw_component::common::update_event_vb::transpile_ue;
use crate::backends::verilog::hw_component::util_vb::{reg_width, sensitivity_list};
use crate::model::common::identifier::Identifiable;
use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::hcp_assign::HcpAssignable;
use crate::model::hw_component::wire::Wire;
use crate::model::model_arena::ModelArena;

impl HcpBaseVb for Wire {
    fn gen_type         (&self) -> String { format!("wire {}", reg_width(self.get_des_slice().get_size())) }
    fn gen_var_name     (&self) -> String { self.get_global_name().to_string() }
    fn amt_init_line    (&self) -> u32    { 1 }
    fn amt_precedure_blk(&self) -> u32    { 1 }

    fn gen_init_line    (&self, _idx: u32, _arena: &mut ModelArena) -> String { String::new() }

    // Wires are always combinational regardless of what clock mode UEs carry.
    fn gen_procedure_blk(&self, _idx: u32, arena: &mut ModelArena) -> String {
        let pool = self.get_update_pool();
        if pool.get_update_events().is_empty() { return String::new(); }

        let sens      = sensitivity_list(ClockMode::ClkFree);
        let tmpl      = format!("{}{}", self.gen_var_name(), "{DES_SLICE} <= {SRC_VAL}{SRC_SLICE};");
        let mut body  = String::new();

        for &ue_i in pool.get_update_events() {
            body += &transpile_ue(ue_i, vec![tmpl.clone()], 4, arena);
        }

        format!("always @({}) begin\n{}end\n", sens, body)
    }

    fn replace_back_into_arena_vb(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_wire(*self); }
}
