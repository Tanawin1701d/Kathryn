// Python-facing HWC arena operations. Mirrors the host `arena_impl_hwc.rs`
// higher-level helpers (those beyond plain CRUD) that Python needs to drive
// model construction.

use pyo3::prelude::*;
use super::model_arena::PyModelArena;
use super::hw_component::common::hcp_ident::PyHcpIdent;
use super::hw_component::common::slice::PySlice;

#[pymethods]
impl PyModelArena {
    // Build a basic assignment node `des_i <= src_i` and attach it to the active
    // flow block (or the top module if none is building). Slices optional
    // (default = full signal); returns nothing, mirroring the host signature.
    #[pyo3(signature = (des_i, src_i, des_slice=None, src_slice=None))]
    fn gen_basic_assign(
        &mut self,
        des_i    : PyHcpIdent,
        src_i    : PyHcpIdent,
        des_slice: Option<PySlice>,
        src_slice: Option<PySlice>,
    ) {
        self.arena.gen_basic_assign(
            des_i.into(),
            src_i.into(),
            des_slice.map(Into::into),
            src_slice.map(Into::into).unwrap_or_default(),
        );
    }
}
