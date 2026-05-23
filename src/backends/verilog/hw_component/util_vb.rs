use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::slice::Slice;

// ---- Width / Slice helpers ----

/// "[N-1:0] " for multi-bit, "" for 1-bit (caller appends signal name).
pub fn reg_width(size: i32) -> String {
    if size <= 1 { String::new() } else { format!("[{}:0] ", size - 1) }
}

/// "[stop-1:start]" for an explicit Slice, "" for the default {-1,-1} (full width).
pub fn slice_to_verilog(s: &Slice) -> String {
    if s.start < 0 { String::new() } else { format!("[{}:{}]", s.stop - 1, s.start) }
}

// ---- Sensitivity list ----

/// Content string for `always @(...)`.
/// PosEdge → "posedge clk", NegEdge → "negedge clk", everything else → "*".
pub fn sensitivity_list(clk_mode: ClockMode) -> String {
    match clk_mode {
        ClockMode::PosEdge => "posedge clk".to_string(),
        ClockMode::NegEdge => "negedge clk".to_string(),
        _                  => "*".to_string(),
    }
}
