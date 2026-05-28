use crate::backends::verilog::hw_component::common::hcp_base_vb::HcpBaseVb;
use crate::backends::verilog::hw_component::util_vb::signal_width;
use crate::model::common::identifier::Identifiable;
use crate::model::hw_component::mem_blk::MemBlk;
use crate::model::model_arena::ModelArena;
use crate::util::file::file_writer::FileWriter;

impl HcpBaseVb for MemBlk {
    fn gen_type_vb    (&self) -> String { let w = signal_width(self.get_bit_width()); format!("reg {w}") }
    fn gen_var_name_vb(&self) -> String { self.get_global_name().to_string() }
    fn amt_init_line_vb(&self) -> u32   { 1 }

    /// Full 2-D reg declaration: `reg [W-1:0] name [0:depth];`
    fn gen_init_line_vb(&self, _idx: u32, _arena: &mut ModelArena, fw: &mut FileWriter) {
        let ty    = self.gen_type_vb();
        let name  = self.gen_var_name_vb();
        let depth = (1i64 << self.get_index_width()) - 1;
        fw.write(&format!("{ty} {name} [0:{depth}];"));
    }

    fn replace_back_into_arena_vb(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_mem_blk(*self); }
}
