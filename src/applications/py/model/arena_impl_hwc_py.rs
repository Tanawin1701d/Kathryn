// Python-facing HWC arena operations. Mirrors the host `arena_impl_hwc.rs`
// higher-level helpers (those beyond plain CRUD) that Python needs to drive
// model construction.

use pyo3::prelude::*;
use super::model_arena::PyModelArena;
use super::hw_component::common::hcp_ident_py::PyHcpIdent;
use super::hw_component::common::slice_py::PySlice;

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

    // Stamp an HCP as an IO port (direction + user-facing name).
    fn mark_as_io(&mut self, hcp_i: PyHcpIdent, is_input: bool, io_name: String) {
        self.arena.mark_as_io(hcp_i.into(), is_input, io_name);
    }

    // True if the HCP has been marked as an IO port.
    fn is_marked_as_io(&self, hcp_i: PyHcpIdent) -> bool {
        self.arena.is_marked_as_io(&hcp_i.into())
    }
}
