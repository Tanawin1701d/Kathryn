use crate::model::common::identifier::Identifiable;
use crate::model::complex_hardware::karray::karray::Karray;
use crate::model::complex_hardware::karray::karray_hw_build::{binary_index_eq, onehot_select_bit};
use crate::model::complex_hardware::karray::karray_meta::index_width_for;
use crate::model::complex_hardware::karray::karray_static_sel::KarrayAsmErr;
use crate::model::complex_hardware::karray::DynRdWrDim;
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::hw_component::common::slice::Slice;
use crate::model::model_arena::ModelArena;

// ===== Karray dynamic (runtime-signal) element WRITE =========================
//
// The write mirror of the dynamic READ (`karray_dynamic_get.rs`). A runtime
// index — binary (`DynBin`) or one-hot (`DynOneHot`) — selects which element
// receives `src`; non-selected elements simply hold. Because the backing is a
// register (clocked), "hold" is free: each element's update event is guarded by
// its own write-enable, and an event that does not fire keeps its value — so this
// is NOT a mux-with-feedback (which a combinational wire would require, hence the
// rejection of `*=`). One guarded `AssignMeta` is emitted per (element, field) and
// all are joined into a single basic node (single-cycle advance), exactly like the
// static element assign (`karray_static_assign.rs::static_index_assign_hcps`).
//
// The per-element write-enable cannot be applied at construction time (a clocked
// target's clk_src is still None then). It is parked on the AssignMeta via
// `set_pending_pre_cond` and folded in at build time, after the clk is wired
// (see `AsmNode::assign_from_state_node` / `dry_assign`).
//
// This file owns the auto-index `dynamic_assign_hcps` fan-out. The shared
// dynamic-write primitives it and the callback-driven `cus_dynamic_assign` both use
// (`gen_element_asm_metas` / `assert_dynamic_write_backing` / `match_sources_to_fields`)
// live in `karray_hw_build.rs`, alongside the family-wide single-node join
// `Karray::attach_metas_as_node`. The cus_dynamic_assign types/driver live in
// `karray_dynamic_cus_assign.rs` / `_run.rs`.

impl Karray {

    // ---- dynamic_assign: auto write-enable from the index --------------------

    /// Write `sources` (each `(field_name, src_i)`) into the runtime-selected
    /// element. `indices` is one `DynRdWrDim` per dimension (Static pins; DynBin /
    /// DynOneHot select at runtime). Reg-backed and clocked only — `clocked == false`
    /// (a `*=`, or a `=` onto a non-clocked backing) or any non-Reg backing is a
    /// `Type` error. Returns the source names that matched no field (caller may warn).
    pub fn dynamic_assign_hcps(
        &self,
        indices : &[DynRdWrDim],
        sources : &[(String, HcpIdent)],
        clocked : bool,
        arena   : &mut ModelArena,
    ) -> Result<Vec<String>, KarrayAsmErr> {
        // step1: reject combinational intent / non-Reg backing (no hold on a wire).
        self.assert_dynamic_write_backing(clocked)?;

        // step2: one index selector per dimension.
        if indices.len() != self.get_dim_count() {
            return Err(KarrayAsmErr::Value(format!(
                "dynamic Karray assign: expected {} index selectors (one per dimension), got {}",
                self.get_dim_count(), indices.len())));
        }

        // step3: resolve field names -> (field_idx, src_i); names with no field are skipped.
        let (matched_srcs, skipped_names) = self.match_sources_to_fields(sources);

        // step4: fan out over the dynamic dims, gathering one guarded AssignMeta per (element, field).
        let mut fixed_indices = Vec::with_capacity(self.get_dim_count());
        let mut metas         = Vec::new();
        self.dynamic_assign_dim(0, &mut fixed_indices, indices, None, &matched_srcs, &mut metas, arena);

        // step5: join the guarded writes into one single-cycle node, return the skipped names.
        self.attach_metas_as_node("dyn_asm", metas, arena);
        Ok(skipped_names)
    }

    // Recursive fan-out mirroring `dynamic_index_get_dim`: Static pins and descends; a
    // dynamic dim loops every index, ANDs that index's write-enable into `we_acc`,
    // and recurses. At a fully-pinned coordinate it emits the guarded writes.
    fn dynamic_assign_dim(
        &self,
        dim_idx      : usize,
        fixed_indices: &mut Vec<usize>,
        abs_indices  : &[DynRdWrDim],
        we_acc       : Option<HcpIdent>,
        resolved_src : &[(usize, HcpIdent)],
        metas        : &mut Vec<AssignMeta>,
        arena        : &mut ModelArena,
    ) {
        // ---- base case: every dim pinned -> one guarded write per matched field ----
        if dim_idx == self.get_dim_count() {
            self.gen_element_asm_metas(fixed_indices, resolved_src, we_acc, metas, arena);
            return;
        }

        // ---- this dim's selector: Static pins; a dynamic dim fans out + ANDs a we ----
        let len = self.get_shape()[dim_idx];
        match abs_indices[dim_idx] {
            DynRdWrDim::Static(fixed_idx) => {
                fixed_indices.push(fixed_idx);
                self.dynamic_assign_dim(dim_idx + 1, fixed_indices, abs_indices, we_acc, resolved_src, metas, arena);
                fixed_indices.pop();
            }
            DynRdWrDim::DynBin(sig_i) => {
                let need = index_width_for(len);
                let got  = arena.get_hw_bit_sz(&sig_i);
                assert!(got >= need,
                    "dynamic_assign: binary selector for dim {dim_idx} needs >= {need} bits to index {len} elements, got {got}");
                for i in 0..len {
                    let we_i = binary_index_eq(sig_i, i, len, arena);
                    let we   = and_we(we_acc, we_i, arena);
                    fixed_indices.push(i);
                    self.dynamic_assign_dim(dim_idx + 1, fixed_indices, abs_indices, we, resolved_src, metas, arena);
                    fixed_indices.pop();
                }
            }
            DynRdWrDim::DynOneHot(sig_i) => {
                let got = arena.get_hw_bit_sz(&sig_i);
                assert!(got >= len as i32,
                    "dynamic_assign: one-hot selector for dim {dim_idx} needs >= {len} bits, got {got}");
                for i in 0..len {
                    let we_i = onehot_select_bit(sig_i, i, arena);
                    let we   = and_we(we_acc, we_i, arena);
                    fixed_indices.push(i);
                    self.dynamic_assign_dim(dim_idx + 1, fixed_indices, abs_indices, we, resolved_src, metas, arena);
                    fixed_indices.pop();
                }
            }
        }
    }
}

// AND a new per-index write-enable bit into the running accumulator (1-bit result).
fn and_we(we_acc  : Option<HcpIdent>,
          new_cond: HcpIdent,
          arena   : &mut ModelArena) -> Option<HcpIdent> {
    match we_acc {
        None       => Some(new_cond),
        Some(prev) => Some(arena.make_expression(false, &format!("{}_WE", new_cond.get_global_name()),
                                                 LogicOp::BitwiseAnd, prev, new_cond, Some(Slice::new(0, 1)), Some(Slice::new(0, 1)))),
    }
}
