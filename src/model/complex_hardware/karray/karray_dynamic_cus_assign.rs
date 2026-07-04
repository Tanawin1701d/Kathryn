use crate::model::complex_hardware::karray::karray::Karray;
use crate::model::complex_hardware::karray::karray_dyn_sel::DynWrCusDim;
use crate::model::complex_hardware::karray::karray_static_sel::KarrayAsmErr;
use crate::model::hw_component::common::hcp_ident::HcpIdent;

// ===== Karray cus_dynamic_assign — types + arena building blocks ==============
//
// The custom callback-driven dynamic WRITE (the write analogue of `karray_dynamic_reduce_get`).
// Where `dynamic_assign` derives each element's write-enable from the index encoding,
// here the CALLER decides each element's enable from the element's static coordinate
// (and any runtime signal they close over). This file holds the validation/resolution
// building block (`write_cus_prepare`); the per-dimension `DynWrCusDim` plan lives in
// `karray_dyn_sel.rs`, and the `WriteEnv` trait and the recursive `write_run` driver
// live in `karray_dynamic_cus_assign_run.rs`.
// The guarded-write primitive it commits through (`gen_element_asm_metas`), the
// shared guards (`assert_dynamic_write_backing` / `match_sources_to_fields`), and the
// family-wide join-into-one-node (`Karray::attach_metas_as_node`) all live in
// `karray_hw_build.rs`.

impl Karray {
    /// Validate a cus_dynamic_assign request: clocked intent + reg backing, one
    /// selector per dim, in-bounds pins, then resolve field names. Returns
    /// `(shape, resolved_sources, skipped_names)` for the recursive driver.
    pub fn write_cus_prepare(
        &self,
        dim_sels: &[DynWrCusDim],
        sources : &[(String, HcpIdent)],
        clocked : bool,
    ) -> Result<(Vec<usize>, Vec<(usize, HcpIdent)>, Vec<String>), KarrayAsmErr> {
        self.assert_dynamic_write_backing(clocked)?;
        if dim_sels.len() != self.get_dim_count() {
            return Err(KarrayAsmErr::Value(format!(
                "cus_dynamic_assign: expected {} dim selectors (one per dimension), got {}",
                self.get_dim_count(), dim_sels.len())));
        }
        for (d, sel) in dim_sels.iter().enumerate() {
            let dim_sz = self.get_shape()[d];
            match sel {
                DynWrCusDim::Pin(i) => if *i >= dim_sz {
                    return Err(KarrayAsmErr::Value(format!(
                        "cus_dynamic_assign: pinned index {i} out of bounds for dim {d} of size {dim_sz}")));
                },
                DynWrCusDim::Range { start, stop } => {
                    let stop = stop.unwrap_or(dim_sz);
                    if *start >= stop || stop > dim_sz {
                        return Err(KarrayAsmErr::Value(format!(
                            "cus_dynamic_assign: empty/invalid range [{start}:{stop}) for dim {d} of size {dim_sz}")));
                    }
                },
                DynWrCusDim::Spread => {}
            }
        }
        let (resolved, skipped) = self.match_sources_to_fields(sources);
        Ok((self.get_shape().clone(), resolved, skipped))
    }
}
