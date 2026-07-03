use crate::model::common::identifier::Identifiable;
use crate::model::complex_hardware::karray::karray::Karray;
use crate::model::complex_hardware::karray::karray_dynamic_get::{bin_eq, onehot_bit};
use crate::model::complex_hardware::karray::karray_meta::index_width_for;
use crate::model::complex_hardware::karray::karray_region_sel::KarrayAsmErr;
use crate::model::complex_hardware::karray::KyIdxType;
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HwComponentType};
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
// static element assign (`karray_static_index.rs::assign_element`).
//
// The per-element write-enable cannot be applied at construction time (a clocked
// target's clk_src is still None then). It is parked on the AssignMeta via
// `set_pending_pre_cond` and folded in at build time, after the clk is wired
// (see `AsmNode::assign_from_state_node` / `dry_assign`).
//
// This file owns the auto-index `dynamic_assign` plus the shared dynamic-write
// PRIMITIVES (`commit_guarded_element` / `assert_dynamic_write_backing` /
// `resolve_write_sources`) that the callback-driven `cus_dynamic_assign` building
// blocks reuse — mirroring how the dynamic-read file owns the mux primitives reduce
// reuses. Joining the gathered metas into one node is done via the family-wide
// `Karray::attach_metas_as_node` (in `karray_static_index.rs`). The cus_dynamic_assign
// types/driver live in `karray_dynamic_cus_assign.rs` / `_run.rs`.

impl Karray {

    // ---- dynamic_assign: auto write-enable from the index --------------------

    /// Write `sources` (each `(field_name, src_i)`) into the runtime-selected
    /// element. `indices` is one `KyIdxType` per dimension (Static pins; DynBin /
    /// DynOneHot select at runtime). Reg-backed and clocked only — `clocked == false`
    /// (a `*=`, or a `=` onto a non-clocked backing) or any non-Reg backing is a
    /// `Type` error. Returns the source names that matched no field (caller may warn).
    pub fn dynamic_assign_element(
        &self,
        indices : &[KyIdxType],
        sources : &[(String, HcpIdent)],
        clocked : bool,
        arena   : &mut ModelArena,
    ) -> Result<Vec<String>, KarrayAsmErr> {
        // step1: reject combinational intent / non-Reg backing (no hold on a wire).
        self.assert_dynamic_write_backing(clocked)?;

        // step2: one index selector per dimension.
        if indices.len() != self.get_dim_size() {
            return Err(KarrayAsmErr::Value(format!(
                "dynamic Karray assign: expected {} index selectors (one per dimension), got {}",
                self.get_dim_size(), indices.len())));
        }

        // step3: resolve field names -> (field_idx, src_i); names with no field are skipped.
        let (matched_srcs, skipped_names) = self.resolve_write_sources(sources);

        // step4: fan out over the dynamic dims, gathering one guarded AssignMeta per (element, field).
        let mut fixed_indices = Vec::with_capacity(self.get_dim_size());
        let mut metas         = Vec::new();
        self.dynamic_assign_base(0, &mut fixed_indices, indices, None, &matched_srcs, &mut metas, arena);

        // step5: join the guarded writes into one single-cycle node, return the skipped names.
        self.attach_metas_as_node("dyn_asm", metas, arena);
        Ok(skipped_names)
    }

    // Recursive fan-out mirroring `dynamic_index_base`: Static pins and descends; a
    // dynamic dim loops every index, ANDs that index's write-enable into `we_acc`,
    // and recurses. At a fully-pinned coordinate it emits the guarded writes.
    fn dynamic_assign_base(
        &self,
        dim_idx      : usize,
        fixed_indices: &mut Vec<usize>,
        abs_indices  : &[KyIdxType],
        we_acc       : Option<HcpIdent>,
        resolved_src : &[(usize, HcpIdent)],
        metas        : &mut Vec<AssignMeta>,
        arena        : &mut ModelArena,
    ) {
        // ---- base case: every dim pinned -> one guarded write per matched field ----
        if dim_idx == self.get_dim_size() {
            self.gen_dyn_asm_meta(fixed_indices, resolved_src, we_acc, metas, arena);
            return;
        }

        // ---- this dim's selector: Static pins; a dynamic dim fans out + ANDs a we ----
        let len = self.get_shape()[dim_idx];
        match abs_indices[dim_idx] {
            KyIdxType::Static(fixed_idx) => {
                fixed_indices.push(fixed_idx);
                self.dynamic_assign_base(dim_idx + 1, fixed_indices, abs_indices, we_acc, resolved_src, metas, arena);
                fixed_indices.pop();
            }
            KyIdxType::DynBin(sig_i) => {
                let need = index_width_for(len);
                let got  = arena.get_hw_bit_sz(&sig_i);
                assert!(got >= need,
                    "dynamic_assign: binary selector for dim {dim_idx} needs >= {need} bits to index {len} elements, got {got}");
                for i in 0..len {
                    let we_i = bin_eq(sig_i, i, len, arena);
                    let we   = and_we(we_acc, we_i, arena);
                    fixed_indices.push(i);
                    self.dynamic_assign_base(dim_idx + 1, fixed_indices, abs_indices, we, resolved_src, metas, arena);
                    fixed_indices.pop();
                }
            }
            KyIdxType::DynOneHot(sig_i) => {
                let got = arena.get_hw_bit_sz(&sig_i);
                assert!(got >= len as i32,
                    "dynamic_assign: one-hot selector for dim {dim_idx} needs >= {len} bits, got {got}");
                for i in 0..len {
                    let we_i = onehot_bit(sig_i, i, arena);
                    let we   = and_we(we_acc, we_i, arena);
                    fixed_indices.push(i);
                    self.dynamic_assign_base(dim_idx + 1, fixed_indices, abs_indices, we, resolved_src, metas, arena);
                    fixed_indices.pop();
                }
            }
        }
    }

    // ---- shared dynamic-write primitives (also used by cus_dynamic_assign) ----

    /// Emit the guarded writes for ONE fully-pinned element `coord`: one
    /// `AssignMeta` per matched field, each driving the field's HCP from its source
    /// and (when `we` is given) parking that write-enable for build-time guarding.
    /// Pushes them onto `metas` (joined into a single node later).
    pub fn gen_dyn_asm_meta(
        &self,
        coord       : &[usize],
        resolved_src: &[(usize, HcpIdent)],
        we          : Option<HcpIdent>,
        metas       : &mut Vec<AssignMeta>,
        arena       : &mut ModelArena,
    ) {
        for &(field_idx, src_i) in resolved_src {
            let des_i             = self.static_index_get_hcp(coord, field_idx, false, arena);
            let src_full_sl = arena.get_hw_slice(&src_i);
            let (mut am, _resize) = arena.gen_asm_meta(des_i, src_i, None, src_full_sl);
            if let Some(c) = we { am.set_pending_pre_cond(c); }
            metas.push(am);
        }
    }

    /// Reject combinational intent and any non-Reg backing for a dynamic write.
    /// `clocked` is the resolved operator intent (`|=` → true, `*=` → false, `=` →
    /// the backing's own clocked-ness), so a `false` here means a combinational write.
    pub(crate) fn assert_dynamic_write_backing(&self, clocked: bool) -> Result<(), KarrayAsmErr> {
        if !clocked {
            return Err(KarrayAsmErr::Type(
                "combinational dynamic Karray write is not supported; use `|=` on a reg-backed Karray".into()));
        }
        if !matches!(self.get_backing(), HwComponentType::Reg) {
            return Err(KarrayAsmErr::Type(format!(
                "dynamic Karray assignment requires a reg-backed Karray, got {:?}", self.get_backing())));
        }
        Ok(())
    }

    /// Resolve `(field_name, src_i)` sources to `(field_idx, src_i)`; names matching
    /// no field are collected for a caller warning.
    pub(crate) fn resolve_write_sources(&self, sources: &[(String, HcpIdent)]) -> (Vec<(usize, HcpIdent)>, Vec<String>) {
        let mut resolved = Vec::with_capacity(sources.len());
        let mut skipped  = Vec::new();
        for (name, src_i) in sources {
            match self.field_index(name) {
                Some(field_idx) => resolved.push((field_idx, *src_i)),
                None            => skipped.push(name.clone()),
            }
        }
        (resolved, skipped)
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
