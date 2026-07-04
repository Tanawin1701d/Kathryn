use crate::backends::verilog::hw_component::common::hcp_base_vb::HcpBaseVb;
use crate::backends::verilog::hw_component::common::update_event_vb::transpile_ue;
use crate::backends::verilog::hw_component::util_vb::{fmt_operand, sensitivity_list, signal_width};
use crate::model::common::identifier::Identifiable;
use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::hcp_assign::HcpAssignable;
use crate::model::hw_component::mem_ele::MemEle;
use crate::model::model_arena::ModelArena;
use crate::util::file::file_writer::FileWriter;

impl HcpBaseVb for MemEle {
    fn gen_type_vb         (&self) -> String { let w = signal_width(self.get_des_slice().get_size()); format!("wire {w}") }
    fn gen_var_name_vb     (&self) -> String { self.get_global_name().to_string() }
    // Only a read port owns a net (`assign name = mem[idx]`); a write port writes
    // straight into the memory array, so its name never appears as a net.
    fn amt_init_line_vb    (&self) -> u32    { if self.is_read() { 1 } else { 0 } }
    fn amt_precedure_blk_vb(&self) -> u32    { 1 }

    /// Full declaration: `wire [N-1:0] name;` (read mode only).
    fn gen_init_line_vb(&self, _idx: u32, _arena: &mut ModelArena, fw: &mut FileWriter) {
        let t = self.gen_type_vb();
        let n = self.gen_var_name_vb();
        fw.write(&format!("{t} {n};"));
    }

    fn gen_procedure_blk_vb(&self, _idx: u32, arena: &mut ModelArena, fw: &mut FileWriter) {
        let name        = self.gen_var_name_vb();
        let master_name = self.get_master_mem_blk_i().get_global_name().to_string();
        let index_name  = self.get_index_ident().get_global_name().to_string();

        // ---- read mode: combinational assign ----
        if self.is_read() {
            fw.write(&format!("assign {name} = {master_name}[{index_name}];\n"));
            return;
        }

        // ---- write mode: clocked always block driven by UE pool ----
        let pool = self.get_update_pool();
        if pool.get_update_events().is_empty() { return; }

        let active_i    = self.get_ident();
        let active_name = self.gen_var_name_vb();
        let clk_mode    = pool.get_clock_mode(arena).unwrap_or(ClockMode::ClkFree);
        // Resolve the real per-module clock source name; fmt_operand guards self-reference.
        let clk_name    = pool.get_clk_src_i(arena)
                              .map(|clk_i| fmt_operand(clk_i, None, arena, active_i, &active_name));
        let sens        = sensitivity_list(clk_mode, clk_name.as_deref());

        // {DES_SLICE} always resolves to "" (full-word write); {SRC} filled by transpile_ue.
        let tmpl = format!("{master_name}[{index_name}]{{DES_SLICE}} <= {{SRC}};");

        fw.write(&format!("always @({sens}) begin\n"));
        for &ue_i in pool.get_update_events() {
            transpile_ue(ue_i, vec![tmpl.clone()], 4, arena, active_i, &active_name, fw);
        }
        fw.write("end\n");
    }

    fn replace_back_into_arena_vb(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_mem_ele(*self); }
}
