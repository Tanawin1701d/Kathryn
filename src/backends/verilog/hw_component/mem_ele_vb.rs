use crate::backends::verilog::hw_component::common::hcp_base_vb::HcpBaseVb;
use crate::backends::verilog::hw_component::common::update_event_vb::transpile_ue;
use crate::backends::verilog::hw_component::util_vb::{sensitivity_list, signal_width};
use crate::model::common::identifier::Identifiable;
use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::hcp_assign::HcpAssignable;
use crate::model::hw_component::memEle::MemEle;
use crate::model::model_arena::ModelArena;
use crate::util::file::file_writer::FileWriter;

impl HcpBaseVb for MemEle {
    fn gen_type         (&self) -> String { let w = signal_width(self.get_des_slice().get_size()); format!("wire {w}") }
    fn gen_var_name     (&self) -> String { self.get_global_name().to_string() }
    fn amt_precedure_blk(&self) -> u32    { 1 }

    fn gen_procedure_blk(&self, _idx: u32, arena: &mut ModelArena, fw: &mut FileWriter) {
        let name        = self.gen_var_name();
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
        let active_name = self.gen_var_name();
        let clk_mode    = pool.get_clock_mode(arena).unwrap_or(ClockMode::ClkFree);
        let sens        = sensitivity_list(clk_mode);

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
