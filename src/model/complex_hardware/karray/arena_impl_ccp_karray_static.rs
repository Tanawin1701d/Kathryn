use crate::model::complex_hardware::common::ccp_ident::CcpIdent;
use crate::model::complex_hardware::karray::karray_static_sel::{KarrayAsmErr, StaticRdWrDim};
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::model_arena::ModelArena;

// Higher-level Karray arena operations for STATIC (compile-time) indexing: resolve
// one field's HCP, multi-field element assignment, and region-to-region (k2k) copy.
// These are thin PROXIES — they take the Karray(s) out of the arena and delegate to
// the matching `Karray::*` method (in karray_static_get.rs / karray_static_assign.rs)
// which owns the actual logic. Dynamic-index ops live in arena_impl_ccp_karray_dynamic.rs.

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
    // is the operator intent — true for `|=`, false for `*=` (a mismatch with the
    // backing is a Type error). Returns the source names that matched no field.
    pub fn karray_static_index_assign_hcps(
        &mut self,
        karray_i      : CcpIdent,
        indices       : &[usize],
        sources       : &[(String, HcpIdent)],
        expect_clocked: bool,
    ) -> Result<Vec<String>, KarrayAsmErr> {
        let karray = self.take_karray(karray_i);
        let out = karray.check_operator_guard(expect_clocked)
            .map(|()| karray.static_index_assign_hcps(indices, sources, self));
        self.replace_back_karray(karray);
        out
    }

    // Region-to-region (k2k) copy proxy: take dst out — reusing it as the source too
    // when both sides are the same Karray (a single take avoids the double-take debug
    // assert) — and delegate to `Karray::static_index_assign_k2k` for the actual logic.
    pub fn karray_static_index_assign_k2k(
        &mut self,
        dst_i         : CcpIdent,
        dst_sel       : Vec<StaticRdWrDim>,
        src_i         : CcpIdent,
        src_sel       : Vec<StaticRdWrDim>,
        expect_clocked: bool,
    ) -> Result<Vec<String>, KarrayAsmErr> {
        let dst       = self.take_karray(dst_i);
        let src_owned = if src_i == dst_i { None } else { Some(self.take_karray(src_i)) };
        let src_ref   = src_owned.as_ref().unwrap_or(&dst);

        let out = dst.static_index_assign_k2k(src_ref, &dst_sel, &src_sel, expect_clocked, self);

        if let Some(src) = src_owned { self.replace_back_karray(src); }
        self.replace_back_karray(dst);
        out
    }
}
