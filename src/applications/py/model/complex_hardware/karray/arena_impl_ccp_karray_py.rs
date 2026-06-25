// Python-facing Karray operations. Mirrors the host `karray/arena_impl_ccp_karray.rs`
// (field resolution + element / region assignment). The factory `mk_karray` lives in
// the shared `arena_factory_ccp_py.rs`.

use pyo3::prelude::*;
use pyo3::exceptions::{PyTypeError, PyValueError};
use super::super::super::model_arena::PyModelArena;
use super::super::super::hw_component::common::hcp_ident_py::PyHcpIdent;
use super::super::ccp_ident_py::PyCcpIdent;
use crate::model::complex_hardware::karray::KarrayAsmErr;
use crate::model::hw_component::common::hcp_ident::HcpIdent;

#[pymethods]
impl PyModelArena {
    // Resolve a static index + field NAME to that field's own HCP (the `|=` / `*=`
    // target). `is_read` only matters for MemBlock backing (read vs write MemEle).
    // The Python handle keeps no field layout — the name is looked up here.
    fn karray_static_index_get_hcp(&mut self, karray_i: PyCcpIdent, indices: Vec<usize>, field: &str, is_read: bool) -> PyResult<PyHcpIdent> {
        self.arena.karray_static_index_get_hcp(karray_i.into(), &indices, field, is_read)
            .map(Into::into)
            .ok_or_else(|| PyValueError::new_err(format!("Karray has no field '{field}'")))
    }

    // Multi-source element assignment: `sources` pairs each field name with its own
    // source HCP, connected full-width (no bit-level split of a packed source).
    // `expect_clocked` is the caller's operator intent — Some(true) for `|=`,
    // Some(false) for `*=` (mismatch with the backing raises TypeError), None for an
    // explicit `=` (no operator guard). Source names matching no field raise a warning.
    #[pyo3(signature = (karray_i, indices, sources, expect_clocked=None))]
    fn karray_static_index_assign_element(
        &mut self,
        py            : Python<'_>,
        karray_i      : PyCcpIdent,
        indices       : Vec<usize>,
        sources       : Vec<(String, PyHcpIdent)>,
        expect_clocked: Option<bool>,
    ) -> PyResult<()> {
        let sources: Vec<(String, HcpIdent)> =
            sources.into_iter().map(|(name, src_i)| (name, src_i.into())).collect();
        match self.arena.karray_static_index_assign_element(karray_i.into(), &indices, &sources, expect_clocked) {
            Ok(skipped) => {
                if !skipped.is_empty() {
                    let msg = format!("karray element assign: skipped sources with no matching field: {skipped:?}");
                    py.import("warnings")?.call_method1("warn", (msg,))?;
                }
                Ok(())
            }
            Err(KarrayAsmErr::Type (m)) => Err(PyTypeError ::new_err(m)),
            Err(KarrayAsmErr::Value(m)) => Err(PyValueError::new_err(m)),
        }
    }

    // Karray-to-karray region assignment. Each side passes its per-dimension
    // selectors as `(start, stop, is_range)` (int → (i, i+1, false); slice →
    // (start, stop|None, true); trailing dims are expanded full-range in Rust).
    // The two selected regions must have equal result shapes; fields are paired by
    // exact name+width, and any skipped destination fields raise a Python warning.
    #[pyo3(signature = (dst_i, dst_sel, src_i, src_sel, expect_clocked=None))]
    fn karray_static_index_assign_k2k(
        &mut self,
        py            : Python<'_>,
        dst_i         : PyCcpIdent,
        dst_sel       : Vec<(usize, Option<usize>, bool)>,
        src_i         : PyCcpIdent,
        src_sel       : Vec<(usize, Option<usize>, bool)>,
        expect_clocked: Option<bool>,
    ) -> PyResult<()> {
        match self.arena.karray_static_index_assign_k2k(dst_i.into(), dst_sel, src_i.into(), src_sel, expect_clocked) {
            Ok(skipped) => {
                if !skipped.is_empty() {
                    let msg = format!(
                        "karray-to-karray assign: skipped destination fields with no name+width match: {skipped:?}");
                    py.import("warnings")?.call_method1("warn", (msg,))?;
                }
                Ok(())
            }
            Err(KarrayAsmErr::Type (m)) => Err(PyTypeError ::new_err(m)),
            Err(KarrayAsmErr::Value(m)) => Err(PyValueError::new_err(m)),
        }
    }
}
