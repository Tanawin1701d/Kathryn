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
    /// Number of the io line
    fn amt_io_line      (&self) -> u32{ 0 }
    /// Number of one-off initialisation lines (e.g. inside `initial` blocks).
    fn amt_init_line    (&self) -> u32{ 0 }
    /// Number of procedural always-blocks this signal emits.
    fn amt_precedure_blk(&self) -> u32{ 0 }

    // ---- combinational generation ----
    /// Emit io declaration line — must be overridden by IO-capable HCP types.
    fn gen_io_line(&self, _idx: u32, _arena: &mut ModelArena) -> String {
        panic!("gen_io_line not implemented for this HCP type")
    }
    /// Emit the `idx`-th initialisation line.
    fn gen_init_line    (&self, idx: u32, arena: &mut ModelArena) -> String{
        panic!("gen_init_line not implemented for this HCP type")
    }
    /// Emit the `idx`-th procedural always-block (including sensitivity list).
    /// Expected to call the update-event transpilation for UE-pool types.
    fn gen_procedure_blk(&self, idx: u32, arena: &mut ModelArena) -> String{
        panic!("gen_procedure_blk not implemented for this HCP type")
    }

    // ---- arena round-trip — each concrete type puts itself back into the correct slot ----
    fn replace_back_into_arena_vb(self: Box<Self>, arena: &mut ModelArena);
}
