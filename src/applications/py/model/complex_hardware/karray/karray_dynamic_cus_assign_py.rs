// Python mirror of `complex_hardware::karray::karray_dynamic_cus_assign_run`. The
// fan-out ALGORITHM lives in the host (`write_run`); this file wires it to Python: the
// `karray_cus_dynamic_assign` pymethod sets things up, and `PyWriteEnv` implements
// `WriteEnv` over the arena. Mirrors `karray_dynamic_reduce_get_py.rs`.

use pyo3::prelude::*;
use pyo3::exceptions::{PyTypeError, PyValueError};
use super::super::super::model_arena::PyModelArena;
use super::super::super::hw_component::common::hcp_ident_py::PyHcpIdent;
use super::super::ccp_ident_py::PyCcpIdent;
use crate::model::complex_hardware::common::ccp_ident::CcpIdent;
use crate::model::complex_hardware::karray::{write_run, KarrayAsmErr, WriteDim, WriteEnv};
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::hcp_ident::HcpIdent;

#[pymethods]
impl PyModelArena {
    // Custom callback-driven dynamic assign. The ALGORITHM lives in the host
    // (`karray_dynamic_assign_run::write_run`); this method wires it to Python via the
    // `WriteEnv` impl below. `dims`: Some(i) pins a dimension, None spreads it (fans
    // out). `raw_fn` is a Python callable `(coord: list[int]) -> write_enable` returning
    // a 1-bit signal that gates element `coord`'s write of `sources`. Reg-backed +
    // clocked only — `expect_clocked == Some(false)` is a TypeError.
    //
    // Takes `slf: &Bound<..>` (NOT &mut self) so the arena pyclass is not borrowed for
    // the method's duration: every WriteEnv arena op borrows in a SCOPED block, and the
    // borrow is dropped before each `enable` callback runs — so the callback may re-enter
    // the arena to build its write-enable expression without an "already borrowed" error.
    fn karray_cus_dynamic_assign(
        slf     : &Bound<'_, PyModelArena>,
        py      : Python<'_>,
        karray_i: PyCcpIdent,
        dims    : Vec<Option<usize>>,
        raw_fn  : Py<PyAny>,
        sources : Vec<(String, PyHcpIdent)>,
        clocked : bool,
    ) -> PyResult<()> {
        let dim_sels: Vec<WriteDim> = dims.into_iter()
            .map(|d| match d { Some(i) => WriteDim::Pin(i), None => WriteDim::Spread })
            .collect();
        let karray_i: CcpIdent = karray_i.into();
        let sources: Vec<(String, HcpIdent)> =
            sources.into_iter().map(|(name, src_i)| (name, src_i.into())).collect();

        // validate + resolve once: the shape (to fan over) and the field-resolved sources.
        let (shape, resolved_src) = {
            let mut me = slf.borrow_mut();
            let karray   = me.arena.take_karray(karray_i);
            let prepared = karray.write_prepare(&dim_sels, &sources, clocked);
            me.arena.replace_back_karray(karray);
            match prepared {
                Ok((shape, resolved, skipped)) => {
                    if !skipped.is_empty() {
                        let msg = format!("cus_dynamic_assign: skipped sources with no matching field: {skipped:?}");
                        py.import("warnings")?.call_method1("warn", (msg,))?;
                    }
                    (shape, resolved)
                }
                Err(KarrayAsmErr::Type (m)) => return Err(PyTypeError ::new_err(m)),
                Err(KarrayAsmErr::Value(m)) => return Err(PyValueError::new_err(m)),
            }
        };

        let mut env = PyWriteEnv { slf, karray_i, raw_fn, resolved_src, metas: Vec::new() };
        write_run(&mut env, &dim_sels, &shape)
    }
}

// WriteEnv over the Python arena: `commit` / `finalize` take a SCOPED borrow; `enable`
// calls the user's Python callback with no borrow held (so it may re-enter the arena to
// build its write-enable expression).
struct PyWriteEnv<'a, 'py> {
    slf         : &'a Bound<'py, PyModelArena>,
    karray_i    : CcpIdent,
    raw_fn      : Py<PyAny>,
    resolved_src: Vec<(usize, HcpIdent)>,     // selected fields resolved to (field_idx, src_i)
    metas       : Vec<AssignMeta>,            // gathered guarded writes, joined at finalize
}

impl WriteEnv for PyWriteEnv<'_, '_> {
    type Err = PyErr;

    fn callback_user_enable(&mut self, coord: &[usize]) -> PyResult<HcpIdent> {
        let py = self.slf.py();
        let we: PyHcpIdent = self.raw_fn.bind(py).call1((coord.to_vec(),))?.extract()?;
        Ok(we.into())
    }

    fn gen_user_asm_meta(&mut self, coord: &[usize], we: HcpIdent) -> PyResult<()> {
        let mut me = self.slf.borrow_mut();
        let karray = me.arena.take_karray(self.karray_i);
        karray.gen_dyn_asm_meta(coord, &self.resolved_src, Some(we), &mut self.metas, &mut me.arena);
        me.arena.replace_back_karray(karray);
        Ok(())
    }

    fn gen_and_attach_asm_node(&mut self) -> PyResult<()> {
        let metas  = std::mem::take(&mut self.metas);
        let mut me = self.slf.borrow_mut();
        let karray = me.arena.take_karray(self.karray_i);
        karray.attach_metas_as_node("dyn_asm", metas, &mut me.arena);
        me.arena.replace_back_karray(karray);
        Ok(())
    }
}
