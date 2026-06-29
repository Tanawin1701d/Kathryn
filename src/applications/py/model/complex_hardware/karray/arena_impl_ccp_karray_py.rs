// Python-facing Karray operations. Mirrors the host `karray/arena_impl_ccp_karray.rs`
// (field resolution + element / region assignment). The factory `mk_karray` lives in
// the shared `arena_factory_ccp_py.rs`.

use pyo3::prelude::*;
use pyo3::exceptions::{PyTypeError, PyValueError};
use super::super::super::model_arena::PyModelArena;
use super::super::super::hw_component::common::hcp_ident_py::PyHcpIdent;
use super::super::ccp_ident_py::PyCcpIdent;
use crate::model::complex_hardware::karray::{KarrayAsmErr, KyIdxType, ReduceDim, ReduceNode};
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
        // ---- decode Python selectors into KyIdxType ----
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

    // Generic callback-driven reduce. `dims` carries one per-dimension selector
    // (Some(i) = pin, None = fold/reduce); `selected_fields` are the field names to
    // carry; `select_fn` is a Python callable invoked per compared pair as
    //   select_fn(a_indices, a_fields, b_indices, b_fields, level) -> HcpIdent (1-bit)
    // where *_fields is a list of (name, HcpIdent). Returns the winning element as a
    // wire-backed scalar Karray CcpIdent.
    //
    // Takes `slf: &Bound<..>` (NOT &mut self) so the arena pyclass is NOT borrowed for
    // the method's duration: each primitive borrows in a SCOPED block and the borrow
    // is dropped before `select_fn` runs, so the callback may re-enter the arena to
    // build its select expression without an "already borrowed" error.
    fn karray_reduce(
        slf            : &Bound<'_, PyModelArena>,
        karray_i       : PyCcpIdent,
        dims           : Vec<Option<usize>>,
        selected_fields: Vec<String>,
        select_fn      : Py<PyAny>,
    ) -> PyResult<PyCcpIdent> {
        let py = slf.py();
        let dims_r: Vec<ReduceDim> = dims.into_iter()
            .map(|d| match d { Some(i) => ReduceDim::Pin(i), None => ReduceDim::Fold })
            .collect();

        // ---- fan out the leaves (scoped borrow) ----
        let mut nodes: Vec<ReduceNode> = {
            let mut me = slf.borrow_mut();
            me.arena.karray_reduce_leaves(karray_i.into(), dims_r, selected_fields)
                .map_err(|e| match e {
                    KarrayAsmErr::Type (m) => PyTypeError ::new_err(m),
                    KarrayAsmErr::Value(m) => PyValueError::new_err(m),
                })?
        };

        // ---- balanced pairwise reduction; select is the user callback ----
        let mut level = 0u32;
        while nodes.len() > 1 {
            let mut next = Vec::with_capacity(nodes.len().div_ceil(2));
            let mut iter = nodes.into_iter();
            while let Some(a) = iter.next() {
                match iter.next() {
                    Some(b) => {
                        // select-left: call Python with NO arena borrow held
                        let args        = (fields_to_py(&a.fields), a.indices.clone(),
                                                fields_to_py(&b.fields), b.indices.clone(), level);
                        let select_left = select_fn.bind(py).call1(args)?.extract::<PyHcpIdent>()?;
                        // mux the pair (scoped borrow)
                        let merged = {
                            let mut me = slf.borrow_mut();
                            me.arena.karray_reduce_mux(karray_i.into(), &a, &b, select_left.into())
                        };
                        next.push(merged);
                    }
                    None => next.push(a),   // odd one out — carried up unchanged
                }
            }
            nodes  = next;
            level += 1;
        }
        let winner = nodes.pop().expect("a reduce has at least one leaf element");

        // ---- pack the winner into a scalar Karray (scoped borrow) ----
        let res_ccp = {
            let mut me = slf.borrow_mut();
            me.arena.karray_reduce_finish(karray_i.into(), &winner)
        };
        Ok(res_ccp.into())
    }
}

// Convert a reduce node's (name, HcpIdent) fields into the Python-facing
// (name, PyHcpIdent) list passed to the select callback.
fn fields_to_py(fields: &[(String, HcpIdent)]) -> Vec<(String, PyHcpIdent)> {
    fields.iter().map(|(name, hcp)| (name.clone(), (*hcp).into())).collect()
}
