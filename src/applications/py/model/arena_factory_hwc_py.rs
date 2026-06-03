// Python-facing hardware-component factories. Mirrors the host
// `arena_factory*.rs` split — HWC `mk_*` constructors live here in their own
// `#[pymethods]` block (enabled by PyO3's `multiple-pymethods` feature).

use pyo3::prelude::*;
use super::model_arena::PyModelArena;
use super::hw_component::common::hcp_ident::PyHcpIdent;

#[pymethods]
impl PyModelArena {
    // Declare a user register `bit_width` bits wide; returns its handle.
    fn mk_reg(&mut self, name: &str, bit_width: i32) -> PyHcpIdent {
        self.arena.make_reg(true, name, bit_width).into()
    }
}
