use crate::backends::verilog::hw_component::common::hcp_base_vb::HcpBaseVb;
use crate::backends::verilog::hw_component::util_vb::reg_width;
use crate::model::common::identifier::Identifiable;
use crate::model::hw_component::common::hcp_assign::HcpAssignable;
use crate::model::hw_component::memBlk::MemBlk;
use crate::model::hw_component::memEle::MemEle;
use crate::model::model_arena::ModelArena;

// ---- MemBlk ----

impl HcpBaseVb for MemBlk {
    fn gen_type(&self) -> String { format!("reg {}", reg_width(self.get_bit_width())) }

    /// Depth index is appended here so the emitter writes `{type} {name}` verbatim.
    fn gen_var_name(&self) -> String {
        let depth = (1i64 << self.get_index_width()) - 1;
        format!("{} [{}:0]", self.get_global_name(), depth)
    }

    fn amt_init_line    (&self) -> u32 { 1 }
    fn amt_precedure_blk(&self) -> u32 { 1 }

    fn gen_init_line    (&self, _idx: u32, _arena: &mut ModelArena) -> String { String::new() }
    fn gen_procedure_blk(&self, _idx: u32, _arena: &mut ModelArena) -> String {
        // TODO: transpile memory UpdatePool UEs into an always block
        String::new()
    }

    fn replace_back_into_arena_vb(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_mem_blk(*self); }
}

// ---- MemEle (MemBlockIndexer) ----

impl HcpBaseVb for MemEle {
    fn gen_type         (&self) -> String { format!("wire {}", reg_width(self.get_des_slice().get_size())) }
    fn gen_var_name     (&self) -> String { self.get_global_name().to_string() }
    fn amt_init_line    (&self) -> u32    { 1 }
    fn amt_precedure_blk(&self) -> u32    { 1 }

    fn gen_init_line    (&self, _idx: u32, _arena: &mut ModelArena) -> String { String::new() }
    fn gen_procedure_blk(&self, _idx: u32, _arena: &mut ModelArena) -> String {
        // TODO: emit assign mem[index] for the indexed read/write accessor
        String::new()
    }

    fn replace_back_into_arena_vb(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_mem_ele(*self); }
}
