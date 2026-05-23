use crate::backends::verilog::hw_component::common::hcp_base_vb::HcpBaseVb;
use crate::backends::verilog::hw_component::common::update_event_vb::transpile_ue;
use crate::backends::verilog::hw_component::util_vb::{reg_width, sensitivity_list};
use crate::model::common::identifier::Identifiable;
use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::hcp_assign::HcpAssignable;
use crate::model::hw_component::sp_reg::cnt_reg::CntReg;
use crate::model::hw_component::sp_reg::state_reg::StateReg;
use crate::model::hw_component::sp_reg::sync_reg::SyncReg;
use crate::model::hw_component::sp_reg::wait_reg::{CondWaitStateReg, CycleWaitStateReg};
use crate::model::model_arena::ModelArena;

// ---- StateReg (1-bit) ----

impl HcpBaseVb for StateReg {
    fn gen_type         (&self) -> String { "reg".to_string() }
    fn gen_var_name     (&self) -> String { self.get_global_name().to_string() }
    fn amt_init_line    (&self) -> u32    { 1 }
    fn amt_precedure_blk(&self) -> u32    { 1 }

    fn gen_init_line    (&self, _idx: u32, _arena: &mut ModelArena) -> String { String::new() }

    fn gen_procedure_blk(&self, _idx: u32, arena: &mut ModelArena) -> String {
        gen_clocked_blk(self, arena)
    }

    fn replace_back_into_arena_vb(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_state_reg(*self); }
}

// ---- SyncReg ----

impl HcpBaseVb for SyncReg {
    fn gen_type         (&self) -> String { format!("reg {}", reg_width(self.get_des_slice().get_size())) }
    fn gen_var_name     (&self) -> String { self.get_global_name().to_string() }
    fn amt_init_line    (&self) -> u32    { 1 }
    fn amt_precedure_blk(&self) -> u32    { 1 }

    fn gen_init_line    (&self, _idx: u32, _arena: &mut ModelArena) -> String { String::new() }

    fn gen_procedure_blk(&self, _idx: u32, arena: &mut ModelArena) -> String {
        gen_clocked_blk(self, arena)
    }

    fn replace_back_into_arena_vb(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_sync_reg(*self); }
}

// ---- CntReg ----

impl HcpBaseVb for CntReg {
    fn gen_type         (&self) -> String { format!("reg {}", reg_width(self.get_cnt_bit_sz())) }
    fn gen_var_name     (&self) -> String { self.get_global_name().to_string() }
    fn amt_init_line    (&self) -> u32    { 1 }
    fn amt_precedure_blk(&self) -> u32    { 1 }

    fn gen_init_line    (&self, _idx: u32, _arena: &mut ModelArena) -> String { String::new() }

    fn gen_procedure_blk(&self, _idx: u32, arena: &mut ModelArena) -> String {
        gen_clocked_blk(self, arena)
    }

    fn replace_back_into_arena_vb(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_cnt_reg(*self); }
}

// ---- CondWaitStateReg (1-bit) ----

impl HcpBaseVb for CondWaitStateReg {
    fn gen_type         (&self) -> String { "reg".to_string() }
    fn gen_var_name     (&self) -> String { self.get_global_name().to_string() }
    fn amt_init_line    (&self) -> u32    { 1 }
    fn amt_precedure_blk(&self) -> u32    { 1 }

    fn gen_init_line    (&self, _idx: u32, _arena: &mut ModelArena) -> String { String::new() }

    fn gen_procedure_blk(&self, _idx: u32, arena: &mut ModelArena) -> String {
        gen_clocked_blk(self, arena)
    }

    fn replace_back_into_arena_vb(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_cond_wait_reg(*self); }
}

// ---- CycleWaitStateReg ----

impl HcpBaseVb for CycleWaitStateReg {
    fn gen_type         (&self) -> String { format!("reg {}", reg_width(self.get_total_bit_size())) }
    fn gen_var_name     (&self) -> String { self.get_global_name().to_string() }
    fn amt_init_line    (&self) -> u32    { 1 }
    fn amt_precedure_blk(&self) -> u32    { 1 }

    fn gen_init_line    (&self, _idx: u32, _arena: &mut ModelArena) -> String { String::new() }

    fn gen_procedure_blk(&self, _idx: u32, arena: &mut ModelArena) -> String {
        gen_clocked_blk(self, arena)
    }

    fn replace_back_into_arena_vb(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_cycle_wait_reg(*self); }
}

// ---- Shared helper ----

/// Build an `always @(sensitivity) begin ... end` block from the UE pool.
/// Used by all clocked sp_reg types whose pool determines the clock mode.
fn gen_clocked_blk(hcp: &(impl HcpBaseVb + HcpAssignable), arena: &mut ModelArena) -> String {
    let pool = hcp.get_update_pool();
    if pool.get_update_events().is_empty() { return String::new(); }

    let clk_mode  = pool.get_clock_mode(arena).unwrap_or(ClockMode::ClkFree);
    let sens      = sensitivity_list(clk_mode);
    let tmpl      = format!("{}{}", hcp.gen_var_name(), "{DES_SLICE} <= {SRC_VAL}{SRC_SLICE};");
    let mut body  = String::new();

    for &ue_i in pool.get_update_events() {
        body += &transpile_ue(ue_i, vec![tmpl.clone()], 4, arena);
    }

    format!("always @({}) begin\n{}end\n", sens, body)
}
