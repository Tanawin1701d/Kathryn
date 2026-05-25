use crate::backends::verilog::hw_component::common::hcp_base_vb::HcpBaseVb;
use crate::backends::verilog::hw_component::util_vb::signal_width;
use crate::model::common::identifier::Identifiable;
use crate::model::hw_component::memBlk::MemBlk;
use crate::model::model_arena::ModelArena;
use crate::util::file::file_writer::FileWriter;

impl HcpBaseVb for MemBlk {
    fn gen_type    (&self) -> String { let w = signal_width(self.get_bit_width()); format!("reg {w}") }
    fn gen_var_name(&self) -> String { self.get_global_name().to_string() }
    fn amt_init_line(&self) -> u32   { 1 }

    /// Writes the depth dimension suffix, e.g. `[255:0]`, appended to the declaration line.
    fn gen_init_line(&self, _idx: u32, _arena: &mut ModelArena, fw: &mut FileWriter) {
        let depth = (1i64 << self.get_index_width()) - 1;
        fw.write(&format!("[{depth}:0]"));
    }

    fn replace_back_into_arena_vb(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_mem_blk(*self); }
}
