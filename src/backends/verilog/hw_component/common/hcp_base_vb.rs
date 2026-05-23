use crate::model::model_arena::ModelArena;

// ---- HcpBaseVb — Verilog-backend capability trait ----
//
// Each HCP type implements this to expose the atomic strings the emitter
// needs.  Two-phase design:
//   1. Declaration — gen_type + gen_var_name → `reg [7:0] foo_reg;`
//   2. Behaviour   — amt_precedure_blk procedural always blocks
//
// `gen_procedure_blk` takes `arena` because it traverses the UpdatePool
// to transpile UE trees.

pub trait HcpBaseVb {
    // ---- atomic query ----

    /// Verilog type keyword + width, e.g. `"reg [7:0]"` or `"wire"`.
    fn gen_type         (&self) -> String;
    /// Signal name as it will appear in emitted Verilog.
    fn gen_var_name     (&self) -> String;

    // ---- count queries (no arena needed) ----

    /// Number of one-off initialisation lines (e.g. inside `initial` blocks).
    fn amt_init_line    (&self) -> u32;
    /// Number of procedural always-blocks this signal emits.
    fn amt_precedure_blk(&self) -> u32;

    // ---- combinational generation ----

    /// Emit the `idx`-th initialisation line.
    fn gen_init_line    (&self, idx: u32, arena: &mut ModelArena) -> String;
    /// Emit the `idx`-th procedural always-block (including sensitivity list).
    /// Expected to call the update-event transpilation for UE-pool types.
    fn gen_procedure_blk(&self, idx: u32, arena: &mut ModelArena) -> String;
}
