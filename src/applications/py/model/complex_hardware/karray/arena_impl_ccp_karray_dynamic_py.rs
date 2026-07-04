// Python-facing Karray DYNAMIC-index operations. Mirrors the host
// `karray/arena_impl_ccp_karray_dynamic.rs` (runtime-signal element read/write and the
// clocked-backing query). Static-index ops live in `arena_impl_ccp_karray_static_py.rs`.

use pyo3::prelude::*;
use pyo3::exceptions::PyValueError;
use super::super::super::model_arena::PyModelArena;
use super::super::super::hw_component::common::hcp_ident_py::PyHcpIdent;
use super::super::ccp_ident_py::PyCcpIdent;
use super::karray_util_py::{karray_err_to_py, warn_skipped_fields};
use crate::model::complex_hardware::karray::DynRdWrDim;
use crate::model::hw_component::common::hcp_ident::HcpIdent;

// Python selector encoding, symmetric for input and the resolved-index output:
//   ("static", Some(i), None)   compile-time index i
//   ("bin",    None,    Some(s))  runtime binary-encoded address signal s
//   ("onehot", None,    Some(s))  runtime one-hot select signal s
type PyKyIdx = (String, Option<usize>, Option<PyHcpIdent>);

#[pymethods]
impl PyModelArena {
    // Dynamic (runtime-signal) element read. `indices` carries one selector per
    // dimension (see PyKyIdx); `selected_fields` are the field names to materialise.
    // Returns the result scalar Karray's CcpIdent plus the resolved per-dimension
    // index (same selector encoding — `bin` wire for dynamic dims, `static` for
    // pinned ones). Reg/Wire backings only.
    fn karray_dynamic_index_get(
        &mut self,
        karray_i       : PyCcpIdent,
        indices        : Vec<PyKyIdx>,
        selected_fields: Vec<String>,
    ) -> PyResult<(PyCcpIdent, Vec<PyKyIdx>)> {
        let sels = decode_selectors(indices)?;

        // ---- resolve + materialise ----
        let (res_ccp, resolved) = self.arena
            .karray_dynamic_index_get(karray_i.into(), sels, selected_fields)
            .map_err(karray_err_to_py)?;

        // ---- encode the resolved index back into Python selectors ----
        let resolved_py: Vec<PyKyIdx> = resolved.into_iter().map(|k| match k {
            DynRdWrDim::Static   (i) => ("static".to_string(), Some(i), None),
            DynRdWrDim::DynBin   (s) => ("bin"   .to_string(), None,    Some(s.into())),
            DynRdWrDim::DynOneHot(s) => ("onehot".to_string(), None,    Some(s.into())),
        }).collect();
        Ok((res_ccp.into(), resolved_py))
    }

    // Whether a Karray's backing is clocked (reg/memblock) vs combinational (wire).
    // The DSL queries this to resolve a bare `=` dynamic write into a concrete flag.
    fn karray_is_clocked(&mut self, karray_i: PyCcpIdent) -> bool {
        self.arena.karray_is_clocked(karray_i.into())
    }

    // Dynamic (runtime-signal) element WRITE: write each `(field_name, src_i)` into
    // the element selected by `indices` (see PyKyIdx). Reg-backed + clocked only —
    // `clocked == false` (a `*=`, or `=` onto a non-clocked backing) raises TypeError.
    // Source names matching no field raise a warning.
    fn karray_dynamic_index_assign_hcps(
        &mut self,
        py      : Python<'_>,
        karray_i: PyCcpIdent,
        indices : Vec<PyKyIdx>,
        sources : Vec<(String, PyHcpIdent)>,
        clocked : bool,
    ) -> PyResult<()> {
        let sels = decode_selectors(indices)?;
        let sources: Vec<(String, HcpIdent)> =
            sources.into_iter().map(|(name, src_i)| (name, src_i.into())).collect();
        let skipped = self.arena
            .karray_dynamic_index_assign_hcps(karray_i.into(), sels, &sources, clocked)
            .map_err(karray_err_to_py)?;
        warn_skipped_fields(py, "karray dynamic assign: skipped sources with no matching field", &skipped)
    }
}

// Decode the Python selector tuples (PyKyIdx) into the host DynRdWrDim, shared by the
// dynamic read and dynamic write paths.
fn decode_selectors(indices: Vec<PyKyIdx>) -> PyResult<Vec<DynRdWrDim>> {
    let mut sels = Vec::with_capacity(indices.len());
    for (kind, sidx, sig) in indices {
        let sel = match kind.as_str() {
            "static" => DynRdWrDim::Static(sidx.ok_or_else(||
                PyValueError::new_err("static index selector needs an integer index"))?),
            "bin"    => DynRdWrDim::DynBin(sig.ok_or_else(||
                PyValueError::new_err("binary index selector needs a signal"))?.into()),
            "onehot" => DynRdWrDim::DynOneHot(sig.ok_or_else(||
                PyValueError::new_err("one-hot index selector needs a signal"))?.into()),
            other    => return Err(PyValueError::new_err(format!("unknown index selector kind '{other}'"))),
        };
        sels.push(sel);
    }
    Ok(sels)
}
