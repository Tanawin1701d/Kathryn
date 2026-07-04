use crate::model::complex_hardware::karray::karray::Karray;
use crate::model::complex_hardware::karray::karray_static_sel::{
    flat_to_multi_index, offset_to_abs_indices, resolve_dim_selectors, result_shape, KarrayAsmErr, StaticRdWrDim,
};
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::slice::Slice;
use crate::model::model_arena::ModelArena;

// ---- Karray static assignment (element multi-source + region-to-region k2k) --

impl Karray {
    // Operator guard shared by every static assign path: `|=` (true) needs a clocked
    // backing, `*=` (false) a combinational one.
    pub fn check_operator_guard(&self, expect_clocked: bool) -> Result<(), KarrayAsmErr> {
        match expect_clocked {
            true  if !self.is_clocked() => Err(KarrayAsmErr::Type(
                "`|=` (clocked assign) requires a reg- or memblock-backed Karray".into())),
            false if  self.is_clocked() => Err(KarrayAsmErr::Type(
                "`*=` (combinational assign) requires a wire-backed Karray".into())),
            _ => Ok(()),
        }
    }

    // Field-matched assignment of one Karray region into another (`self` = dst, `src`
    // = source). The two selected regions must have equal result shapes; fields are
    // paired by exact name+width (unmatched destination fields are skipped and returned
    // for a caller warning). One AssignMeta is generated per (element, matched field)
    // and all are joined into a single basic asm node attached to the current scope.
    // Each side's per-dim selectors are `StaticRdWrDim` — `Index` collapses the dim,
    // `Range` keeps it (see karray_static_sel.rs).
    pub fn static_index_assign_k2k(
        &self,
        src           : &Karray,
        dst_sel       : &[StaticRdWrDim],
        src_sel       : &[StaticRdWrDim],
        expect_clocked: bool,
        arena         : &mut ModelArena,
    ) -> Result<Vec<String>, KarrayAsmErr> {
        let dst = self;

        // Step 1: operator guard — reject `|=` on a comb backing / `*=` on a clocked one.
        dst.check_operator_guard(expect_clocked)?;

        // Step 2: resolve each side's per-dim selectors to concrete index sets, then
        //         require both selected regions to have the same result shape.
        let dst_sels  = resolve_dim_selectors(dst.get_shape(), dst_sel)?;
        let src_sels  = resolve_dim_selectors(src.get_shape(), src_sel)?;
        let dst_sel_shape = result_shape(&dst_sels);
        let src_sel_shape = result_shape(&src_sels);
        if dst_sel_shape != src_sel_shape {
            return Err(KarrayAsmErr::Value(format!(
                "Karray region shape mismatch: dest {dst_sel_shape:?} vs source {src_sel_shape:?}")));
        }

        // Step 3: pair fields by exact name + width. `matched` holds (dst_field_idx,
        //         src_field_idx) pairs; a dst field with no counterpart is skipped and
        //         reported. Zero matches is a hard error (nothing to copy).
        let mut matched: Vec<(usize, usize)> = Vec::new();
        let mut skipped: Vec<String>         = Vec::new();
        for (dfi, des_field) in dst.get_fields().iter().enumerate() {
            let sfi = src.get_fields().iter()
                .position(|src_field| src_field.matches(des_field));
            match sfi {
                Some(sfi) => matched.push((dfi, sfi)),
                None      => skipped.push(des_field.get_name().to_string()),
            }
        }
        if matched.is_empty() {
            return Err(KarrayAsmErr::Value(
                "Karray assignment: no destination field matches a source field by name+width".into()));
        }

        // Step 4: walk every selected element (flat index -> multi-index -> absolute
        //         indices on each side) and, per matched field, resolve both HCPs and
        //         build one AssignMeta connecting src field -> dst field full-width.
        let total: usize           = dst_sel_shape.iter().product();   // 1 when shapes are empty
        let mut metas: Vec<AssignMeta> = Vec::with_capacity(total * matched.len());
        for flat in 0..total {
            let offset_indices = flat_to_multi_index(flat, &dst_sel_shape);
            let dst_indices    = offset_to_abs_indices(&dst_sels, &offset_indices);
            let src_indices    = offset_to_abs_indices(&src_sels, &offset_indices);
            for &(dfi, sfi) in &matched {
                let width       = dst.get_fields()[dfi].get_width();
                let des_i       = dst.static_index_get_hcp(&dst_indices, dfi, false, arena);
                let src_field_i = src.static_index_get_hcp(&src_indices, sfi, true, arena);
                let des         = arena.take_hcp(des_i);
                let am          = des.gen_asm_meta(src_field_i, None, Slice::new(0, width), arena);
                arena.replace_back_hcp(des);
                metas.push(am);
            }
        }

        // Step 5: join all metas into ONE basic asm node (the joined-node rule) and
        //         attach it to the current scope. Return the skipped dst-field names.
        dst.attach_metas_as_node("karr_asm", metas, arena);

        Ok(skipped)
    }

    /// Multi-source element assignment: each `(field_name, src_i)` drives the field
    /// of that name at `indices` from its own source HCP — a full-width connect, with
    /// no bit-level split of a packed source. All field writes are joined into ONE
    /// basic node (so a seq block advances a single cycle, not one cycle per field)
    /// and attached to the current scope. Returns the source names that matched no
    /// field (caller may warn).
    pub fn static_index_assign_hcps(&self, indices: &[usize], sources: &[(String, HcpIdent)], arena: &mut ModelArena) -> Vec<String> {
        let mut metas: Vec<AssignMeta> = Vec::with_capacity(sources.len());
        let mut skipped                = Vec::new();
        for (name, src_i) in sources {
            match self.field_index(name) {
                Some(field_idx) => {
                    let des_i    = self.static_index_get_hcp(indices, field_idx, false, arena);
                    // full-width source, auto-resized to the field; record ONE AssignMeta
                    // (joined into a single node below — not a node per field).
                    let src_full      = arena.get_hw_slice(src_i);
                    let (am, _resize) = arena.gen_asm_meta(des_i, *src_i, None, src_full);
                    metas.push(am);
                }
                None => skipped.push(name.clone()),
            }
        }

        // ---- join all field writes into a single basic node, attach to current scope ----
        // (`attach_metas_as_node` is the family-wide join, now in `karray_hw_build.rs`.)
        self.attach_metas_as_node("elem_asm", metas, arena);
        skipped
    }
}
