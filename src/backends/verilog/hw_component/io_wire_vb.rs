use crate::backends::verilog::hw_component::common::hcp_base_vb::HcpBaseVb;
use crate::backends::verilog::hw_component::util_vb::reg_width;
use crate::model::common::identifier::Identifiable;
use crate::model::hw_component::common::hcp_assign::HcpAssignable;
use crate::model::hw_component::io_wire::IoWire;
use crate::model::model_arena::ModelArena;

impl HcpBaseVb for IoWire {
    fn gen_type         (&self) -> String { format!("wire {}", reg_width(self.get_des_slice().get_size())) }
    fn gen_var_name     (&self) -> String { self.get_global_name().to_string() }
    fn amt_init_line    (&self) -> u32    { 1 }
    fn amt_precedure_blk(&self) -> u32    { 1 }

    fn gen_init_line    (&self, _idx: u32, _arena: &mut ModelArena) -> String { String::new() }
    fn gen_procedure_blk(&self, _idx: u32, _arena: &mut ModelArena) -> String {
        // TODO: emit assign connecting agent_src_signal to this wire
        String::new()
    }
}
