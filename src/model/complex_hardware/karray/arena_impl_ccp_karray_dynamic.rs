use crate::model::complex_hardware::common::ccp_ident::CcpIdent;
use crate::model::complex_hardware::karray::karray_static_sel::KarrayAsmErr;
use crate::model::complex_hardware::karray::DynRdWrDim;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::model_arena::ModelArena;

// Higher-level Karray arena operations for DYNAMIC (runtime-signal) indexing: an
// element read/write where a binary or one-hot signal selects the element at build
// time, plus the backing-clocked query the DSL uses to resolve a dynamic write's
// operator. The reduce / cus_dynamic_assign drivers are NOT here — they run through
// the connector's ReduceEnv / WriteEnv impls (see the note at the bottom).
// Static-index ops (get_hcp / element assign / region k2k) live in
// arena_impl_ccp_karray_static.rs.

impl ModelArena {
    // Dynamic (runtime-signal) element read on `karray_i`: resolve the per-dimension
    // selectors (`DynRdWrDim` — Static / DynBin / DynOneHot) and the named fields into
    // a fresh wire-backed scalar Karray, returning its CcpIdent plus the resolved
    // per-dimension index. Field names are resolved here (Python keeps no layout); an
    // unknown name is a Value error.
    pub fn karray_dynamic_index_get(
        &mut self,
        karray_i   : CcpIdent,
        indices    : Vec<DynRdWrDim>,
        field_names: Vec<String>,
    ) -> Result<(CcpIdent, Vec<DynRdWrDim>), KarrayAsmErr> {
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
            Ok(karray.dynamic_index_get(&indices, field_idxs, self))
        };
        self.replace_back_karray(karray);
        out
    }

    // Whether a Karray's backing is clocked (reg/memblock → `|=`) or combinational
    // (wire → `*=`). Lets the DSL resolve a bare `=` on a dynamic write into a concrete
    // clocked flag from the destination's own backing.
    pub fn karray_is_clocked(&mut self, karray_i: CcpIdent) -> bool {
        let karray = self.take_karray(karray_i);
        let out    = karray.is_clocked();
        self.replace_back_karray(karray);
        out
    }

    // Dynamic (runtime-signal) element WRITE on `karray_i`: write each `(field_name,
    // src_i)` into the element selected by `indices` (`DynRdWrDim` — Static / DynBin /
    // DynOneHot), guarding every element by its own write-enable so non-selected ones
    // hold. Reg-backed + clocked only (`clocked == false` is rejected). Returns the
    // source names that matched no field (caller may warn).
    pub fn karray_dynamic_index_assign_hcps(
        &mut self,
        karray_i: CcpIdent,
        indices : Vec<DynRdWrDim>,
        sources : &[(String, HcpIdent)],
        clocked : bool,
    ) -> Result<Vec<String>, KarrayAsmErr> {
        let karray = self.take_karray(karray_i);
        let out    = karray.dynamic_assign_hcps(&indices, sources, clocked, self);
        self.replace_back_karray(karray);
        out
    }



    // (Reduce and cus_dynamic_assign are driven by `karray_dynamic_reduce_get::reduce_run` /
    // `karray_dynamic_cus_assign_run::write_run` via the connector's ReduceEnv / WriteEnv
    // impls, which call Karray building blocks directly — no per-op arena wrappers here.)
}
