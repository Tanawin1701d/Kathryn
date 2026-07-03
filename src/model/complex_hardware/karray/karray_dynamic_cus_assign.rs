use crate::model::complex_hardware::karray::karray::Karray;
use crate::model::complex_hardware::karray::karray_region_sel::KarrayAsmErr;
use crate::model::hw_component::common::hcp_ident::HcpIdent;

// ===== Karray cus_dynamic_assign — types + arena building blocks ==============
//
// The custom callback-driven dynamic WRITE (the write analogue of `karray_dynamic_reduce_get`).
// Where `dynamic_assign` derives each element's write-enable from the index encoding,
// here the CALLER decides each element's enable from the element's static coordinate
// (and any runtime signal they close over). This file holds the per-dimension TYPE
// and the validation/resolution building block (`write_prepare`); the `WriteEnv`
// trait and the recursive `write_run` driver live in `karray_dynamic_cus_assign_run.rs`.
// The guarded-write primitive it commits through (`commit_guarded_element`) and the
// shared guards (`assert_dynamic_write_backing` / `resolve_write_sources`) are owned
// by `karray_dynamic_assign.rs`; the final join-into-one-node uses the family-wide
// `Karray::attach_metas_as_node` (in `karray_static_index.rs`).

/// Per-dimension plan for `cus_dynamic_assign`: pin to one index, or spread (fan
/// out and let the user's callback decide each element's write-enable).
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum WriteDim {
    Pin   (usize),
    Spread,
}

impl Karray {
    /// Validate a cus_dynamic_assign request: clocked intent + reg backing, one
    /// selector per dim, in-bounds pins, then resolve field names. Returns
    /// `(shape, resolved_sources, skipped_names)` for the recursive driver.
    pub fn write_prepare(
        &self,
        dim_sels: &[WriteDim],
        sources : &[(String, HcpIdent)],
        clocked : bool,
    ) -> Result<(Vec<usize>, Vec<(usize, HcpIdent)>, Vec<String>), KarrayAsmErr> {
        self.assert_dynamic_write_backing(clocked)?;
        if dim_sels.len() != self.get_dim_size() {
            return Err(KarrayAsmErr::Value(format!(
                "cus_dynamic_assign: expected {} dim selectors (one per dimension), got {}",
                self.get_dim_size(), dim_sels.len())));
        }
        for (d, sel) in dim_sels.iter().enumerate() {
            if let WriteDim::Pin(i) = sel {
                if *i >= self.get_shape()[d] {
                    return Err(KarrayAsmErr::Value(format!(
                        "cus_dynamic_assign: pinned index {i} out of bounds for dim {d} of size {}", self.get_shape()[d])));
                }
            }
        }
        let (resolved, skipped) = self.resolve_write_sources(sources);
        Ok((self.get_shape().clone(), resolved, skipped))
    }
}
