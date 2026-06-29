use crate::model::common::identifier::Identifiable;
use crate::model::complex_hardware::common::ccp_ident::CcpIdent;
use crate::model::complex_hardware::karray::Karray;
use crate::model::complex_hardware::karray::karray_region_sel::{
    flat_to_multi_index, offset_to_abs_indices, resolve_dim_selectors, result_shape, KarrayAsmErr,
};
use crate::model::complex_hardware::karray::{KyIdxType, ReduceDim, ReduceNode};
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::slice::Slice;
use crate::model::model_arena::ModelArena;

// Higher-level Karray operation: region-to-region assignment between two Karrays.
// Creation/stamping lives in arena_factory_ccp.rs; CRUD in arena_impl_ccp.rs.
// Per-dimension selector math lives in karray::karray_region_sel.

impl ModelArena {
    // Resolve a static index + field NAME on `karray_i` to that field's own HCP
    // (the `|=` / `*=` target). `is_read` only matters for MemBlock backing (read vs
    // write MemEle). Returns None if the Karray has no field of that name.
    pub fn karray_static_index_get_hcp(&mut self, karray_i: CcpIdent, indices: &[usize], field: &str, is_read: bool) -> Option<HcpIdent> {
        let karray = self.take_karray(karray_i);
        let out    = karray.field_index(field)
            .map(|idx| karray.static_index_get_hcp(indices, idx, is_read, self));
        self.replace_back_karray(karray);
        out
    }

    // Multi-source element assignment on `karray_i`: each `(field_name, src_i)` drives
    // the field of that name (full-width connect, joined into one node). `expect_clocked`
    // is the operator intent — Some(true) for `|=`, Some(false) for `*=` (a mismatch with
    // the backing is a Type error). Returns the source names that matched no field.
    pub fn karray_static_index_assign_element(
        &mut self,
        karray_i      : CcpIdent,
        indices       : &[usize],
        sources       : &[(String, HcpIdent)],
        expect_clocked: Option<bool>,
    ) -> Result<Vec<String>, KarrayAsmErr> {
        let karray = self.take_karray(karray_i);
        let out = match expect_clocked {
            Some(true)  if !karray.is_clocked() => Err(KarrayAsmErr::Type(
                "`|=` (clocked assign) requires a reg- or memblock-backed Karray".into())),
            Some(false) if  karray.is_clocked() => Err(KarrayAsmErr::Type(
                "`*=` (combinational assign) requires a wire-backed Karray".into())),
            _ => Ok(karray.assign_element(indices, sources, self)),
        };
        self.replace_back_karray(karray);
        out
    }

    // Dynamic (runtime-signal) element read on `karray_i`: resolve the per-dimension
    // selectors (`KyIdxType` — Static / DynBin / DynOneHot) and the named fields into
    // a fresh wire-backed scalar Karray, returning its CcpIdent plus the resolved
    // per-dimension index. Field names are resolved here (Python keeps no layout); an
    // unknown name is a Value error.
    pub fn karray_dynamic_index_get(
        &mut self,
        karray_i   : CcpIdent,
        indices    : Vec<KyIdxType>,
        field_names: Vec<String>,
    ) -> Result<(CcpIdent, Vec<KyIdxType>), KarrayAsmErr> {
        let karray = self.take_karray(karray_i);

        // resolve field names -> field indices (collecting any misses for a clean error)
        let mut field_idxs = Vec::with_capacity(field_names.len());
        let mut missing    = Vec::new();
        for name in &field_names {
            match karray.field_index(name) {
                Some(field_idx) => field_idxs.push(field_idx),
                None            => missing.push(name.clone()),
            }
        }

        let out = if !missing.is_empty() {
            Err(KarrayAsmErr::Value(format!("Karray dynamic read: no such field(s): {missing:?}")))
        } else {
            Ok(karray.dynamic_index_get_Karray(&indices, field_idxs, self))
        };
        self.replace_back_karray(karray);
        out
    }

    // ---- generic callback-driven reduce primitives -------------------------
    // The reduce LOOP is driven by the caller (the Python connector, which must
    // release the arena borrow around each user select callback). These are the
    // short take/replace_back primitives it calls between callbacks.

    // Fan out the reduce leaves: one ReduceNode per selected element. Field names
    // are resolved here (an unknown name is a Value error).
    pub fn karray_reduce_leaves(
        &mut self,
        karray_i   : CcpIdent,
        dims       : Vec<ReduceDim>,
        field_names: Vec<String>,
    ) -> Result<Vec<ReduceNode>, KarrayAsmErr> {
        let karray = self.take_karray(karray_i);
        let mut field_idxs = Vec::with_capacity(field_names.len());
        let mut missing    = Vec::new();
        for name in &field_names {
            match karray.field_index(name) {
                Some(field_idx) => field_idxs.push(field_idx),
                None            => missing.push(name.clone()),
            }
        }
        let out = if !missing.is_empty() {
            Err(KarrayAsmErr::Value(format!("Karray reduce: no such field(s): {missing:?}")))
        } else {
            Ok(karray.reduce_leaves(&dims, &field_idxs, self))
        };
        self.replace_back_karray(karray);
        out
    }

    // Mux two reduce subtrees under `select_left`, producing the merged subtree.
    pub fn karray_reduce_mux(
        &mut self,
        karray_i   : CcpIdent,
        a          : &ReduceNode,
        b          : &ReduceNode,
        select_left: HcpIdent,
    ) -> ReduceNode {
        let karray = self.take_karray(karray_i);
        let out    = karray.reduce_mux(a, b, select_left, self);
        self.replace_back_karray(karray);
        out
    }

    // Pack the winning subtree into a fresh wire-backed scalar Karray.
    pub fn karray_reduce_finish(&mut self, karray_i: CcpIdent, winner: &ReduceNode) -> CcpIdent {
        let karray = self.take_karray(karray_i);
        let out    = karray.reduce_finish(winner, self);
        self.replace_back_karray(karray);
        out
    }

    // Field-matched assignment of one Karray region into another. The two selected
    // regions must have equal result shapes; fields are paired by exact name+width
    // (unmatched destination fields are skipped and returned for a caller warning).
    // One AssignMeta is generated per (element, matched field) and all are joined
    // into a single basic asm node attached to the current scope.
    // int  -> (i, i+1, False)   collapse this dim
    // slice-> (start, stop|None, True)   keep this dim
    pub fn karray_static_index_assign_k2k(
        &mut self,
        dst_i         : CcpIdent,
        dst_sel       : Vec<(usize, Option<usize>, bool)>,
        src_i         : CcpIdent,
        src_sel       : Vec<(usize, Option<usize>, bool)>,
        expect_clocked: Option<bool>,
    ) -> Result<Vec<String>, KarrayAsmErr> {
        // Take dst out; reuse it as the source too when both sides are the same
        // Karray (a single take avoids the double-take debug assert).
        let dst       = self.take_karray(dst_i);
        let src_owned = if src_i == dst_i { None } else { Some(self.take_karray(src_i)) };
        let src_ref: &Karray = src_owned.as_ref().unwrap_or(&dst);

        let out = self.karray_static_index_assign_k2k_base(&dst, src_ref, &dst_sel, &src_sel, expect_clocked);

        if let Some(src) = src_owned { self.replace_back_karray(src); }
        self.replace_back_karray(dst);
        out
    }

    // Inner worker: dst/src are already taken out (locals), so the arena (self) is
    // free for static_index_hcp / take_hcp / node construction.
    fn karray_static_index_assign_k2k_base(
        &mut self,
        dst           : &Karray,
        src           : &Karray,
        dst_sel       : &[(usize, Option<usize>, bool)],
        src_sel       : &[(usize, Option<usize>, bool)],
        expect_clocked: Option<bool>,
    ) -> Result<Vec<String>, KarrayAsmErr> {
        // ---- operator guard (|= clocked vs *= combinational) ----
        match expect_clocked {
            Some(true)  if !dst.is_clocked() => return Err(KarrayAsmErr::Type(
                "`|=` (clocked assign) requires a reg- or memblock-backed Karray".into())),
            Some(false) if  dst.is_clocked() => return Err(KarrayAsmErr::Type(
                "`*=` (combinational assign) requires a wire-backed Karray".into())),
            _ => {}
        }

        // ---- resolve selectors + shape match ----
        let dst_sels  = resolve_dim_selectors(dst.get_shape(), dst_sel)?;
        let src_sels  = resolve_dim_selectors(src.get_shape(), src_sel)?;
        let dst_sel_shape = result_shape(&dst_sels);
        let src_sel_shape = result_shape(&src_sels);
        if dst_sel_shape != src_sel_shape {
            return Err(KarrayAsmErr::Value(format!(
                "Karray region shape mismatch: dest {dst_sel_shape:?} vs source {src_sel_shape:?}")));
        }

        // ---- pair fields by exact name + width ----
        let mut matched: Vec<(usize, usize)> = Vec::new(); // it contain index of karray field
        let mut skipped: Vec<String>         = Vec::new();
        for (dfi, des_field) in dst.get_fields().iter().enumerate() {
            let sfi = src.get_fields().iter()
                .position(|src_field|
                              ( src_field.get_name() == des_field.get_name()) &&
                              ( src_field.get_width() == des_field.get_width()));
            match sfi {
                Some(sfi) => matched.push((dfi, sfi)),
                None      => skipped.push(des_field.get_name().to_string()),
            }
        }
        if matched.is_empty() {
            return Err(KarrayAsmErr::Value(
                "Karray assignment: no destination field matches a source field by name+width".into()));
        }

        // ---- one AssignMeta per (element, matched field) ----
        let total: usize           = dst_sel_shape.iter().product();   // 1 when shapes are empty
        let mut metas: Vec<AssignMeta> = Vec::with_capacity(total * matched.len());
        for flat in 0..total {
            let offset_indices = flat_to_multi_index(flat, &dst_sel_shape);
            let dst_indices    = offset_to_abs_indices(&dst_sels, &offset_indices);
            let src_indices    = offset_to_abs_indices(&src_sels, &offset_indices);
            for &(dfi, sfi) in &matched {
                let width       = dst.get_fields()[dfi].get_width();
                let des_i       = dst.static_index_get_hcp(&dst_indices, dfi, false, self);
                let src_field_i = src.static_index_get_hcp(&src_indices, sfi, true, self);
                let des         = self.take_hcp(des_i);
                let am          = des.gen_asm_meta(src_field_i, None, Slice::new(0, width), self);
                self.replace_back_hcp(des);
                metas.push(am);
            }
        }

        // ---- join all metas into a single basic node, attach to current scope ----
        let name   = format!("{}_karr_asm", dst.get_ccp_ident().get_global_name());
        let node_i = self.make_asm_node_many(&name, &metas);
        self.attach_basic_node_to_current_scope(node_i);

        Ok(skipped)
    }
}
