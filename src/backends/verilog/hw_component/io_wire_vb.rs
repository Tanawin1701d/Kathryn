use crate::backends::verilog::hw_component::common::hcp_base_vb::HcpBaseVb;
use crate::backends::verilog::hw_component::util_vb::{gen_procedure_blk, signal_width};
use crate::model::common::identifier::Identifiable;
use crate::model::hw_component::common::hcp_assign::HcpAssignable;
use crate::model::hw_component::io_wire::IoWire;
use crate::model::model_arena::ModelArena;
use crate::util::file::file_writer::FileWriter;

impl HcpBaseVb for IoWire {
    fn gen_type         (&self) -> String { let w = signal_width(self.get_des_slice().get_size()); format!("wire {w}") }
    fn gen_var_name     (&self) -> String { self.get_global_name().to_string() }
    fn amt_io_line      (&self) -> u32    { 1 }
    fn amt_precedure_blk(&self) -> u32    { if self.get_is_input() { 0 } else { 1 } }

    // Emit the Verilog port direction + type, e.g. `output wire [7:0] foo_out`.
    fn gen_io_line(&self, _idx: u32, _arena: &mut ModelArena, fw: &mut FileWriter) {
        let dir  = if self.get_is_input() { "input" } else { "output" };
        let w    = signal_width(self.get_des_slice().get_size());
        let name = self.gen_var_name();
        fw.write(&format!("{dir} wire {w}{name}"));
    }

    fn gen_init_line(&self, _idx: u32, _arena: &mut ModelArena, _fw: &mut FileWriter) { /* IO wires have no internal init declaration */ }

    // Input ports are driven externally — no always block.
    // Output ports emit a combinational always block that routes agent_src_signal.
    fn gen_procedure_blk(&self, _idx: u32, arena: &mut ModelArena, fw: &mut FileWriter) {
        if self.get_is_input() { return; }
        gen_procedure_blk(self, self.get_ident(), arena, fw)
    }

    fn replace_back_into_arena_vb(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_io_wire(*self); }
}
