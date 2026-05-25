use crate::model::model_arena::ModelArena;
use crate::util::file::file_writer::FileWriter;

// ---- HcpBaseVb — Verilog-backend capability trait ----
//
// Each HCP type implements this to expose the atomic strings the emitter
// needs.  Two-phase design:
//   1. Declaration — gen_type_vb + gen_var_name_vb → `reg [7:0] foo_reg;`
//   2. Behaviour   — amt_precedure_blk_vb procedural always blocks
//
// gen_io_line_vb / gen_init_line_vb / gen_procedure_blk_vb write directly
// into the supplied FileWriter instead of returning a String, so the emitter
// never allocates a full-module string.

pub trait HcpBaseVb {

    // ---- atomic query ----

    /// Verilog type keyword + width, e.g. `"reg [7:0]"` or `"wire"`.
    fn gen_type_vb    (&self) -> String;
    /// Signal name as it will appear in emitted Verilog.
    fn gen_var_name_vb(&self) -> String;

    // ---- count queries (no arena needed) ----

    /// Number of IO port declaration lines.
    fn amt_io_line_vb      (&self) -> u32 { 0 }
    /// Number of one-off initialisation lines (e.g. signal declarations).
    fn amt_init_line_vb    (&self) -> u32 { 0 }
    /// Number of procedural always-blocks this signal emits.
    fn amt_precedure_blk_vb(&self) -> u32 { 0 }

    // ---- write-through generation ----

    /// Write the idx-th IO port declaration directly into fw.
    fn gen_io_line_vb(&self, _idx: u32, _arena: &mut ModelArena, _fw: &mut FileWriter) {
        panic!("gen_io_line_vb not implemented for this HCP type")
    }

    /// Write the idx-th initialisation / declaration line directly into fw.
    fn gen_init_line_vb(&self, _idx: u32, _arena: &mut ModelArena, _fw: &mut FileWriter) {
        panic!("gen_init_line_vb not implemented for this HCP type")
    }

    /// Write the idx-th procedural always-block (including sensitivity list) directly into fw.
    fn gen_procedure_blk_vb(&self, _idx: u32, _arena: &mut ModelArena, _fw: &mut FileWriter) {
        panic!("gen_procedure_blk_vb not implemented for this HCP type")
    }

    // ---- arena round-trip — each concrete type puts itself back into the correct slot ----
    fn replace_back_into_arena_vb(self: Box<Self>, arena: &mut ModelArena);
}
