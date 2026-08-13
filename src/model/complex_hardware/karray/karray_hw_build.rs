use crate::model::common::identifier::Identifiable;
use crate::model::complex_hardware::karray::karray::Karray;
use crate::model::complex_hardware::karray::karray_meta::index_width_for;
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::hw_component::common::slice::Slice;
use crate::model::model_arena::ModelArena;

// Shared Karray hardware-build primitives — the one home for wiring helpers used
// by both engines: the mux builders the READ tree folds with (`karray_read.rs`)
// and the write-enable primitives the guarded WRITE fans out with
// (`karray_write.rs`), plus the single-node join every assign path ends in.
// The generic 2:1 comb mux lives in `common/ccp_hw_build.rs` (shared with other
// CCPs) and is re-exported here for the read engine.

// ---- combinational mux builders (read path) ---------------------------------

pub(crate) use crate::model::complex_hardware::common::ccp_hw_build::mux_into_wire;

// Binary per-level select-left bit: at tree level `layer`, the two children of a
// node differ in bit `layer` of the address (left = 0, right = 1), so pick left
// when `sig[layer] == 0`, i.e. `~sig[layer]`.
pub(crate) fn bin_layer_select_left(sig_i: HcpIdent, layer: u32, arena: &mut ModelArena) -> HcpIdent {
    let bit     = layer as i32;
    let bit_sig = arena.make_expression_single(false, &format!("{}_B{layer}", sig_i.get_global_name()),
        LogicOp::SliceBit, sig_i, Some(Slice::new(bit, bit + 1)));
    arena.make_expression_single(false, &format!("{}_B{layer}_N", sig_i.get_global_name()),
        LogicOp::BitwiseInvr, bit_sig, None)
}

// ---- write-enable primitives (write path) -----------------------------------

// Binary equality write-enable for index `idx`: `sig == idx` (a 1-bit signal), with
// the constant sized to the dimension's index width. Enables exactly the
// runtime-selected element of a `Dyn` write dim.
pub(crate) fn binary_index_eq(sig_i: HcpIdent, idx: usize, len: usize, arena: &mut ModelArena) -> HcpIdent {
    let iw      = index_width_for(len);
    let const_i = arena.make_val(false, &format!("{}_EQ{idx}", sig_i.get_global_name()), iw, idx as u64);
    arena.make_expression(false, &format!("{}_EQ{idx}_C", sig_i.get_global_name()),
        LogicOp::RelationEq, sig_i, const_i, None, None)
}

// AND a new per-index write-enable bit into the running accumulator (1-bit result).
pub(crate) fn and_we(we_acc: Option<HcpIdent>, new_cond: HcpIdent, arena: &mut ModelArena) -> Option<HcpIdent> {
    match we_acc {
        None       => Some(new_cond),
        Some(prev) => Some(arena.make_expression(false, &format!("{}_WE", new_cond.get_global_name()),
                                                 LogicOp::BitwiseAnd, prev, new_cond,
                                                 Some(Slice::new(0, 1)), Some(Slice::new(0, 1)))),
    }
}

// ---- shared Karray assign primitives ----------------------------------------

impl Karray {
    /// Join a batch of per-field `AssignMeta`s into a SINGLE basic node named
    /// `<ccp>_<suffix>` and attach it to the current scope (no-op when empty). This is
    /// the one place a Karray turns several field writes into a single-cycle node — so
    /// a seq block advances once, not once per field. Every assign path ends here.
    pub fn attach_metas_as_node(&self, suffix: &str, metas: Vec<AssignMeta>, arena: &mut ModelArena) {
        if !metas.is_empty() {
            let name   = format!("{}_{suffix}", self.get_ccp_ident().get_global_name());
            let node_i = arena.make_asm_node_many(&name, &metas);
            arena.attach_basic_node_to_current_scope(node_i);
        }
    }

    /// Emit the writes for ONE fully-pinned element `coord`: one `AssignMeta` per
    /// `(field_idx, src_i)` source, each driving the field's HCP full-width and
    /// (when `we` is given) parking that write-enable for build-time guarding.
    /// The enable cannot be applied at construction time (a clocked target's
    /// clk_src is still None then); `set_pending_pre_cond` folds it in at build,
    /// after the clk is wired.
    pub(crate) fn gen_element_asm_metas(
        &self,
        // ---- static — one pinned element + its sources and enable ----
        coord       : &[usize],
        resolved_src: &[(usize, HcpIdent)],
        we          : Option<HcpIdent>,
        // ---- sinks — output accumulator + arena context ----
        metas       : &mut Vec<AssignMeta>,
        arena       : &mut ModelArena,
    ) {
        for &(field_idx, src_i) in resolved_src {
            let des_i             = self.element_hcp(coord, field_idx);
            let src_full_sl       = arena.get_hw_slice(&src_i);
            let (mut am, _resize) = arena.gen_asm_meta(des_i, src_i, None, src_full_sl);
            if let Some(cond_i) = we { am.set_pending_pre_cond(cond_i); }
            metas.push(am);
        }
    }
}
