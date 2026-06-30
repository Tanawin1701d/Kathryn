// Python mirror of `complex_hardware::karray::karray_reduce`. The reduce ALGORITHM
// lives in the host (`reduce_run`); this file wires it to Python: the `karray_reduce`
// pymethod sets things up, and `PyReduceEnv` implements `ReduceEnv` over the arena.

use pyo3::prelude::*;
use pyo3::exceptions::{PyTypeError, PyValueError};
use super::super::super::model_arena::PyModelArena;
use super::super::super::hw_component::common::hcp_ident_py::PyHcpIdent;
use super::super::ccp_ident_py::PyCcpIdent;
use crate::model::common::identifier::Identifiable;
use crate::model::complex_hardware::common::ccp_ident::CcpIdent;
use crate::model::complex_hardware::karray::{reduce_mux, reduce_pack, reduce_run, KarrayAsmErr, NamedHcp, ReduceDim, ReduceEnv};
use crate::model::hw_component::common::hcp_ident::HcpIdent;

#[pymethods]
impl PyModelArena {
    // Generic callback-driven reduce. The ALGORITHM lives in the host
    // (`karray_reduce::reduce_run`); this method wires it to Python via the `ReduceEnv`
    // impl below. `dims`: Some(i) pins a dimension, None folds it. `raw_fns` is one entry
    // per dimension (None for pinned) — each a Python callable
    // `(a_fields, a_indices, b_fields, b_indices, level) -> (sel, extras)`. Returns the
    // winner Karray plus the per-folded-dim winner index signals.
    //
    // Takes `slf: &Bound<..>` (NOT &mut self) so the arena pyclass is not borrowed for
    // the method's duration: every ReduceEnv arena op borrows in a SCOPED block, and the
    // borrow is dropped before each `select` callback runs — so the callback may re-enter
    // the arena to build its select expression without an "already borrowed" error.
    fn karray_reduce(
        slf          : &Bound<'_, PyModelArena>,
        karray_i     : PyCcpIdent,
        dims         : Vec<Option<usize>>,
        raw_fns      : Vec<Option<Py<PyAny>>>,
        fields       : Vec<String>,
        request_index: bool,
    ) -> PyResult<(PyCcpIdent, Vec<PyHcpIdent>)> {
        let dim_sels: Vec<ReduceDim> = dims.into_iter()
            .map(|d| match d { Some(i) => ReduceDim::Pin(i), None => ReduceDim::Fold })
            .collect();
        let karray_i: CcpIdent = karray_i.into();

        // validate + resolve once: the shape (to fan over), the field indices, and the
        // result Karray's name.
        let (shape, field_idxs, result_name) = {
            let mut me = slf.borrow_mut();
            let karray   = me.arena.take_karray(karray_i);
            let prepared = karray.reduce_prepare(&fields, &dim_sels);
            let name     = format!("{}_REDUCE", karray.get_ccp_ident().get_global_name());
            me.arena.replace_back_karray(karray);
            match prepared {
                Ok((shape, field_idxs))     => (shape, field_idxs, name),
                Err(KarrayAsmErr::Type (m)) => return Err(PyTypeError ::new_err(m)),
                Err(KarrayAsmErr::Value(m)) => return Err(PyValueError::new_err(m)),
            }
        };

        let mut env = PyReduceEnv { slf, karray_i, result_name, raw_fns, field_idxs };
        let (result, coords) = reduce_run(&mut env, &dim_sels, &shape, &fields, request_index)?;
        Ok((result.into(), coords.into_iter().map(Into::into).collect()))
    }
}

// ReduceEnv over the Python arena: the arena ops (`leaf`/`mux`/`const_index`/`pack`)
// take a SCOPED borrow; `select` calls the user's Python callback with no borrow held
// (so it may re-enter the arena to build its select expression).
struct PyReduceEnv<'a, 'py> {
    slf         : &'a Bound<'py, PyModelArena>,
    karray_i    : CcpIdent,
    result_name : String,
    raw_fns     : Vec<Option<Py<PyAny>>>,     // one per dimension; None for pinned dims
    field_idxs  : Vec<usize>,                 // selected fields, resolved to indices once
}

impl ReduceEnv for PyReduceEnv<'_, '_> {
    type Err = PyErr;

    fn leaf(&mut self, coord: &[usize]) -> PyResult<Vec<NamedHcp>> {
        let mut me = self.slf.borrow_mut();
        let karray = me.arena.take_karray(self.karray_i);
        let out    = karray.reduce_leaf(coord, &self.field_idxs, &mut me.arena);
        me.arena.replace_back_karray(karray);
        Ok(out)
    }

    fn select(&mut self,
              dim   : usize,
              a     : &[NamedHcp], a_at: &[Vec<usize>],
              b     : &[NamedHcp], b_at: &[Vec<usize>],
              level : u32) -> PyResult<(HcpIdent, Vec<NamedHcp>)> {
        let py  = self.slf.py();
        let raw = self.raw_fns.get(dim).and_then(|f| f.as_ref())
            .ok_or_else(|| PyValueError::new_err(format!("reduce: dimension {dim} has no select fn")))?;
        let args = (fields_to_py(a), a_at.to_vec(), fields_to_py(b), b_at.to_vec(), level);
        let (sel, extras): (PyHcpIdent, Vec<(String, PyHcpIdent)>) = raw.bind(py).call1(args)?.extract()?;
        Ok((sel.into(), extras.into_iter().map(|(n, h)| (n, h.into())).collect()))
    }

    fn mux(&mut self, pairs: Vec<(String, HcpIdent, HcpIdent)>, sel: HcpIdent) -> PyResult<Vec<NamedHcp>> {
        let mut me = self.slf.borrow_mut();
        Ok(reduce_mux(&mut me.arena, &pairs, sel))
    }

    fn const_index(&mut self, width: i32, value: usize) -> PyResult<HcpIdent> {
        let mut me = self.slf.borrow_mut();
        Ok(me.arena.make_val(false, &format!("RIDX{value}"), width, value as u64))
    }

    fn pack(&mut self, fields: Vec<NamedHcp>) -> PyResult<CcpIdent> {
        let mut me = self.slf.borrow_mut();
        Ok(reduce_pack(&mut me.arena, &self.result_name, &fields))
    }
}

// Convert (name, HcpIdent) fields into the Python-facing (name, PyHcpIdent) list.
fn fields_to_py(fields: &[NamedHcp]) -> Vec<(String, PyHcpIdent)> {
    fields.iter().map(|(name, hcp)| (name.clone(), (*hcp).into())).collect()
}
