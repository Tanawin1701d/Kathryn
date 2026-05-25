use crate::backends::verilog::hw_component::common::hcp_base_vb::HcpBaseVb;
use crate::backends::verilog::hw_component::util_vb::{gen_procedure_blk, signal_width};
use crate::model::common::identifier::Identifiable;
use crate::model::hw_component::common::hcp_assign::HcpAssignable;
use crate::model::hw_component::wire::Wire;
use crate::model::model_arena::ModelArena;
use crate::util::file::file_writer::FileWriter;

impl HcpBaseVb for Wire {
    fn gen_type         (&self) -> String { let w = signal_width(self.get_des_slice().get_size()); format!("wire {w}") }
    fn gen_var_name     (&self) -> String { self.get_global_name().to_string() }
    fn amt_init_line    (&self) -> u32    { 1 }
    fn amt_precedure_blk(&self) -> u32    { 1 }

    /// Full declaration: `wire [N-1:0] name;`
    fn gen_init_line(&self, _idx: u32, _arena: &mut ModelArena, fw: &mut FileWriter) {
        let t = self.gen_type();
        let n = self.gen_var_name();
        fw.write(&format!("{t} {n};"));
    }

    // Wires are always combinational regardless of what clock mode UEs carry.
    fn gen_procedure_blk(&self, _idx: u32, arena: &mut ModelArena, fw: &mut FileWriter) {
        gen_procedure_blk(self, self.get_ident(), arena, fw)
    }

    fn replace_back_into_arena_vb(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_wire(*self); }
}
