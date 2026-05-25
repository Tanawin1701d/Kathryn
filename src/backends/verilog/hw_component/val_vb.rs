use crate::backends::verilog::hw_component::common::hcp_base_vb::HcpBaseVb;
use crate::backends::verilog::hw_component::util_vb::signal_width;
use crate::model::common::identifier::Identifiable;
use crate::model::hw_component::common::hcp_assign::HcpAssignable;
use crate::model::hw_component::val::Val;
use crate::model::model_arena::ModelArena;
use crate::util::file::file_writer::FileWriter;
use crate::util::math::vary_val::VaryVal;

// ---- literal formatter ----

/// Format a VaryVal as a Verilog sized literal: `N'hVALUE` (hex, multi-bit)
/// or `1'b0` / `1'b1` (binary, 1-bit).  Leading zeros are stripped from the
/// hex digits; at least one digit is always emitted.
fn vary_val_to_verilog(val: &VaryVal) -> String {
    let width = val.bit_width();

    if width == 1 {
        return format!("1'b{}", if val.get_bit(0) { 1 } else { 0 });
    }

    // Limbs are little-endian; reverse for MSB-first hex output.
    let full_hex: String = val.limbs().iter().rev()
        .map(|l| format!("{l:016x}"))
        .collect();

    let trimmed = full_hex.trim_start_matches('0');
    let hex     = if trimmed.is_empty() { "0" } else { trimmed };

    format!("{width}'h{hex}")
}

// ---- HcpBaseVb impl ----

impl HcpBaseVb for Val {
    fn gen_type_vb    (&self) -> String { let w = signal_width(self.get_des_slice().get_size()); format!("localparam {w}") }
    fn gen_var_name_vb(&self) -> String { self.get_global_name().to_string() }
    fn amt_init_line_vb(&self) -> u32   { 1 }

    /// Full declaration line: `localparam [N-1:0] NAME = VALUE;`
    fn gen_init_line_vb(&self, _idx: u32, _arena: &mut ModelArena, fw: &mut FileWriter) {
        let t    = self.gen_type_vb();
        let name = self.gen_var_name_vb();
        let val  = vary_val_to_verilog(self.get_value());
        fw.write(&format!("{t} {name} = {val};"));
    }

    fn replace_back_into_arena_vb(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_val(*self); }
}
