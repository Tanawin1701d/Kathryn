// Python-facing Karray operations. Mirrors the host `karray/arena_impl_ccp_karray.rs`
// (field resolution + element / region assignment). The factory `mk_karray` lives in
// the shared `arena_factory_ccp_py.rs`.

use pyo3::prelude::*;
use pyo3::exceptions::{PyTypeError, PyValueError};
use super::super::super::model_arena::PyModelArena;
use super::super::super::hw_component::common::hcp_ident_py::PyHcpIdent;
use super::super::ccp_ident_py::PyCcpIdent;
use crate::model::complex_hardware::karray::{KarrayAsmErr, KyIdxType};
use crate::model::hw_component::common::hcp_ident::HcpIdent;

// Python selector encoding, symmetric for input and the resolved-index output:
//   ("static", Some(i), None)   compile-time index i
//   ("bin",    None,    Some(s))  runtime binary-encoded address signal s
//   ("onehot", None,    Some(s))  runtime one-hot select signal s
type PyKyIdx = (String, Option<usize>, Option<PyHcpIdent>);

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
            .map_err(|e| match e {
                KarrayAsmErr::Type (m) => PyTypeError ::new_err(m),
                KarrayAsmErr::Value(m) => PyValueError::new_err(m),
            })?;

        // ---- encode the resolved index back into Python selectors ----
        let resolved_py: Vec<PyKyIdx> = resolved.into_iter().map(|k| match k {
            KyIdxType::Static   (i) => ("static".to_string(), Some(i), None),
            KyIdxType::DynBin   (s) => ("bin"   .to_string(), None,    Some(s.into())),
            KyIdxType::DynOneHot(s) => ("onehot".to_string(), None,    Some(s.into())),
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
    fn karray_dynamic_index_assign_element(
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
        match self.arena.karray_dynamic_index_assign_element(karray_i.into(), sels, &sources, clocked) {
            Ok(skipped) => {
                if !skipped.is_empty() {
                    let msg = format!("karray dynamic assign: skipped sources with no matching field: {skipped:?}");
                    py.import("warnings")?.call_method1("warn", (msg,))?;
                }
                Ok(())
            }
            Err(KarrayAsmErr::Type (m)) => Err(PyTypeError ::new_err(m)),
            Err(KarrayAsmErr::Value(m)) => Err(PyValueError::new_err(m)),
        }
    }
}

// Decode the Python selector tuples (PyKyIdx) into the host KyIdxType, shared by the
// dynamic read and dynamic write paths.
fn decode_selectors(indices: Vec<PyKyIdx>) -> PyResult<Vec<KyIdxType>> {
    let mut sels = Vec::with_capacity(indices.len());
    for (kind, sidx, sig) in indices {
        let sel = match kind.as_str() {
            "static" => KyIdxType::Static(sidx.ok_or_else(||
                PyValueError::new_err("static index selector needs an integer index"))?),
            "bin"    => KyIdxType::DynBin(sig.ok_or_else(||
                PyValueError::new_err("binary index selector needs a signal"))?.into()),
            "onehot" => KyIdxType::DynOneHot(sig.ok_or_else(||
                PyValueError::new_err("one-hot index selector needs a signal"))?.into()),
            other    => return Err(PyValueError::new_err(format!("unknown index selector kind '{other}'"))),
        };
        sels.push(sel);
    }
    Ok(sels)
}