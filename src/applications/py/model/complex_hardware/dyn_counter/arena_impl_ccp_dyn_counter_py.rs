// Python-facing DynCounter operations. Mirrors the host
// `dyn_counter/arena_impl_ccp_dyn_counter.rs` (add / update / view accessors on
// an already-created counter). The factory `mk_dyn_counter` lives in the shared
// `arena_factory_ccp_py.rs`; int-literal addends are wrapped into width-matched
// `val`s by the DSL before they reach here.

use pyo3::prelude::*;
use pyo3::exceptions::PyValueError;
use super::super::super::model_arena::PyModelArena;
use super::super::super::hw_component::common::hcp_ident_py::PyHcpIdent;
use super::super::super::hw_component::common::slice_py::PySlice;
use super::super::ccp_ident_py::PyCcpIdent;
use crate::model::hw_component::common::hcp_ident::HcpIdent;

#[pymethods]
impl PyModelArena {
    // Chain one add stage: `enable ? prev + addend : prev` (plain add without an
    // enable). Returns the new chain-head signal.
    #[pyo3(signature = (cnt_i, addend_i, addend_slice=None, enable_i=None, enable_slice=None))]
    fn dyn_counter_add(
        &mut self,
        cnt_i       : PyCcpIdent        ,
        addend_i    : PyHcpIdent        ,
        addend_slice: Option<PySlice>   ,
        enable_i    : Option<PyHcpIdent>,
        enable_slice: Option<PySlice>   ,
    ) -> PyResult<PyHcpIdent> {
        if let Some(en_i) = enable_i {
            let en_i: HcpIdent = en_i.into();
            self.check_cond_slice_match(en_i, enable_slice)?;
        }
        self.arena.dyn_counter_add(
                cnt_i.into(),
                addend_i.into(), addend_slice.map(Into::into),
                enable_i.map(Into::into), enable_slice.map(Into::into))
            .map(Into::into)
            .map_err(PyValueError::new_err)
    }

    // Commit the chain head into the counter register (one clocked statement in
    // the current flow scope); the chain restarts from the register.
    fn dyn_counter_update(&mut self, cnt_i: PyCcpIdent) -> PyResult<()> {
        self.arena.dyn_counter_update(cnt_i.into()).map_err(PyValueError::new_err)
    }

    // ---- view accessors ----------------------------------------------------

    // The counter's backing register (the committed value).
    fn dyn_counter_get_reg(&mut self, cnt_i: PyCcpIdent) -> PyHcpIdent {
        self.arena.dyn_counter_get_reg_i(cnt_i.into()).into()
    }

    // Head of the uncommitted add chain (the register when nothing is pending).
    fn dyn_counter_get_now(&mut self, cnt_i: PyCcpIdent) -> PyHcpIdent {
        self.arena.dyn_counter_get_now_i(cnt_i.into()).into()
    }
}
