// Python-facing Karray STATIC-index operations. Mirrors the host
// `karray/arena_impl_ccp_karray_static.rs` (field resolution + static element and
// region assignment). Dynamic-index ops live in `arena_impl_ccp_karray_dynamic_py.rs`.
// The factory `mk_karray` lives in the shared `arena_factory_ccp_py.rs`.

use pyo3::prelude::*;
use pyo3::exceptions::PyValueError;
use super::super::super::model_arena::PyModelArena;
use super::super::super::hw_component::common::hcp_ident_py::PyHcpIdent;
use super::super::ccp_ident_py::PyCcpIdent;
use super::karray_util_py::{karray_err_to_py, warn_skipped_fields};
use crate::model::complex_hardware::karray::StaticRdWrDim;
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
    // Some(false) for `*=` (mismatch with the backing raises TypeError); None (a bare
    // `=`) is rejected here. Source names matching no field raise a warning.
    #[pyo3(signature = (karray_i, indices, sources, expect_clocked=None))]
    fn karray_static_index_assign_hcps(
        &mut self,
        py            : Python<'_>,
        karray_i      : PyCcpIdent,
        indices       : Vec<usize>,
        sources       : Vec<(String, PyHcpIdent)>,
        expect_clocked: Option<bool>,
    ) -> PyResult<()> {
        let expect_clocked = require_operator(expect_clocked)?;
        let sources: Vec<(String, HcpIdent)> =
            sources.into_iter().map(|(name, src_i)| (name, src_i.into())).collect();
        let skipped = self.arena
            .karray_static_index_assign_hcps(karray_i.into(), &indices, &sources, expect_clocked)
            .map_err(karray_err_to_py)?;
        warn_skipped_fields(py, "karray element assign: skipped sources with no matching field", &skipped)
    }

    // Karray-to-karray region assignment. Each side passes its per-dimension
    // selectors as `(start, stop, is_range)` (int → (i, i+1, false); slice →
    // (start, stop|None, true); trailing dims are expanded full-range in Rust).
    // The two selected regions must have equal result shapes; fields are paired by
    // exact name+width, and any skipped destination fields raise a Python warning.
    // None (a bare `=`) is rejected here — the caller must use `|=` or `*=`.
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
        let expect_clocked = require_operator(expect_clocked)?;
        let skipped = self.arena
            .karray_static_index_assign_k2k(
                dst_i.into(), to_static_dims(dst_sel),
                src_i.into(), to_static_dims(src_sel),
                expect_clocked)
            .map_err(karray_err_to_py)?;
        warn_skipped_fields(py, "karray-to-karray assign: skipped destination fields with no name+width match", &skipped)
    }
}

// Static Karray assignment demands a concrete operator: reject a bare `=` (None) so the
// caller must pick `|=` (clocked) or `*=` (combinational). Shared by both static paths.
fn require_operator(expect_clocked: Option<bool>) -> PyResult<bool> {
    expect_clocked.ok_or_else(||
        PyValueError::new_err("static Karray assignment requires `|=` or `*=`, not a bare `=`"))
}

// Decode the Python per-dim selector tuples `(start, stop, is_range)` into StaticRdWrDim:
// an int key (`is_range == false`) collapses the dim to `Index(start)`; a slice keeps it
// as `Range { start, stop }` (the DSL sends int as `(i, i+1, false)`, slice as `(s, stop|None, true)`).
fn to_static_dims(sel: Vec<(usize, Option<usize>, bool)>) -> Vec<StaticRdWrDim> {
    sel.into_iter()
        .map(|(start, stop, is_range)| if is_range {
            StaticRdWrDim::Range { start, stop }
        } else {
            StaticRdWrDim::Index(start)
        })
        .collect()
}
