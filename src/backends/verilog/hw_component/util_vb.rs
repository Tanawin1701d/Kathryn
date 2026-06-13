use crate::backends::verilog::hw_component::common::hcp_base_vb::HcpBaseVb;
use crate::backends::verilog::hw_component::common::update_event_vb::transpile_ue;
use crate::model::common::identifier::Identifiable;
use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::hcp_assign::HcpAssignable;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::hw_component::common::slice::Slice;
use crate::model::model_arena::ModelArena;
use crate::util::file::file_writer::FileWriter;

// ---- Width / Slice helpers ----

/// "[N-1:0] " for multi-bit, "" for 1-bit (caller appends signal name).
pub fn signal_width(size: i32) -> String {
    let n = size - 1; 
    format!("[{n}:0] ")
}

/// "[stop-1:start]" for an explicit Slice, "" for the default {-1,-1} (full width).
pub fn slice_to_verilog(s: &Slice) -> String {
    if s.start < 0 { String::new() } else { let top = s.stop - 1; let bot = s.start; format!("[{top}:{bot}]") }
}

// ---- LogicOp → Verilog operator string ----

/// Map a model `LogicOp` to its Verilog operator token.
/// This is intentionally separate from `LogicOp::to_op_str` (model layer)
/// because some variants have no direct Verilog operator equivalent
/// (`ExtendBit`, signed relational) and must be caught here at the backend boundary.
pub fn logic_op_to_verilog(op: LogicOp) -> &'static str {
    match op {
        LogicOp::BitwiseAnd  => "&",
        LogicOp::BitwiseOr   => "|",
        LogicOp::BitwiseXor  => "^",
        LogicOp::BitwiseInvr => "~",
        LogicOp::BitwiseShl  => "<<",
        LogicOp::BitwiseShr  => ">>",
        LogicOp::LogicalAnd  => "&&",
        LogicOp::LogicalOr   => "||",
        LogicOp::LogicalNot  => "!",
        LogicOp::RelationEq  => "==",
        LogicOp::RelationNeq => "!=",
        LogicOp::RelationLe  => "<",
        LogicOp::RelationLeq => "<=",
        LogicOp::RelationGe  => ">",
        LogicOp::RelationGeq => ">=",
        // Signed relational: Verilog requires $signed() wrapping on operands;
        // emit the operator token only — caller must supply signed-cast wrappers
        // when the full expression is generated.
        LogicOp::RelationSlt => "<",
        LogicOp::RelationSgt => ">",
        LogicOp::ArithPlus   => "+",
        LogicOp::ArithMinus  => "-",
        LogicOp::ArithMul    => "*",
        LogicOp::ArithDiv    => "/",
        LogicOp::ArithDivr   => "%",
        LogicOp::Assign      => panic!("logic_op_to_verilog: Assign has no operator token; handle as wire-through"),
        LogicOp::ExtendBit   => panic!("logic_op_to_verilog: ExtendBit has no direct Verilog operator; not yet implemented"),
        LogicOp::Dummy       => panic!("logic_op_to_verilog: Dummy op must not reach the Verilog backend"),
    }
}

// ---- variable declaration ----
pub fn fmt_init_var(var_type: String, var_name: String) -> String { 
    format!("{var_type}{var_name};")
}

// ---- Operand formatter ----

/// Format one resolved operand as `"var_name[slice]"` (or just `"var_name"` for full-width).
/// Resolves the Verilog name via `gen_var_name()` so per-type overrides are respected.
///
/// `active_i` / `active_name`: the HCP the caller is currently processing — already taken
/// from the arena, so it cannot be re-taken.  If `opr` is that same HCP (self-reference),
/// `active_name` is used directly instead of an arena round-trip.
pub fn fmt_operand(
    opr         : HcpIdent,
    slice       : Option<Slice>,
    arena       : &mut ModelArena,
    active_i    : HcpIdent,
    active_name : &str,
) -> String {
    let var_name = if opr.get_global_id() == active_i.get_global_id() {
        // Self-reference: HCP is already out of the arena — use the provided name.
        active_name.to_string()
    } else {
        let vb   = arena.take_hcp_vb(opr);
        let name = vb.gen_var_name_vb();
        arena.replace_back_hcp_vb(vb);
        name
    };
    let sl = slice_to_verilog(&slice.unwrap_or_default());
    format!("{var_name}{sl}")
}

// ---- Sensitivity list ----

/// Content string for `always @(...)`.
/// PosEdge → "posedge <clk>", NegEdge → "negedge <clk>", everything else → "*".
/// `clk_name` is the per-module Verilog name of the event's clock source (resolved
/// from `UpdatePool::get_clk_src_i`). There is **no** `"clk"` fallback: every
/// clocked event sees its clock via its own IoWire (e.g. `IO_IN_clk`), so an edge
/// mode with `clk_name == None` is a wiring bug and panics rather than guessing.
pub fn sensitivity_list(clk_mode: ClockMode, clk_name: Option<&str>) -> String {
    // No default clk: an edge mode with no resolved clk source is a wiring bug
    // (e.g. a clocked HCP whose clk_node_i was never assigned) — fail loudly
    // rather than silently emit a bare `clk` that may not be the right net.
    match clk_mode {
        ClockMode::PosEdge => format!("posedge {}", clk_name.expect("sensitivity_list: PosEdge with no clk source")),
        ClockMode::NegEdge => format!("negedge {}", clk_name.expect("sensitivity_list: NegEdge with no clk source")),
        _                  => "*".to_string(),
    }
}

// ---- Clocked always-block builder ----

/// Write an `always @(sensitivity) begin … end` block from an HCP's UE pool into fw.
/// Shared by `Reg`, all sp_reg types, and any other clocked HCP that uses the
/// standard `{DES_SLICE} <= {SRC}` assignment template.
///
/// `active_i` must be `self.get_ident()` from the caller so `fmt_operand` can
/// detect a self-referential source and avoid a double arena-take.
pub fn gen_procedure_blk(
    hcp      : &(impl HcpBaseVb + HcpAssignable),
    active_i : HcpIdent,
    arena    : &mut ModelArena,
    fw       : &mut FileWriter,
) {
    let pool = hcp.get_update_pool();
    if pool.get_update_events().is_empty() { return; }

    let active_name = hcp.gen_var_name_vb();
    let clk_mode    = pool.get_clock_mode(arena).unwrap_or(ClockMode::ClkFree);
    // Resolve the real per-module clock source name; fmt_operand guards self-reference.
    let clk_name    = pool.get_clk_src_i(arena)
                          .map(|clk_i| fmt_operand(clk_i, None, arena, active_i, &active_name));
    let sens        = sensitivity_list(clk_mode, clk_name.as_deref());
    let tmpl        = format!("{active_name}{{DES_SLICE}} <= {{SRC}};");

    fw.write(&format!("always @({sens}) begin\n"));
    for &ue_i in pool.get_update_events() {
        transpile_ue(ue_i, vec![tmpl.clone()], 4, arena, active_i, &active_name, fw);
    }
    fw.write("end\n");
}
