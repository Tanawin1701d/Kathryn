// Python-facing top-module / build ops. Mirrors the top-module and build
// helpers in the host `arena_impl.rs`.

use pyo3::prelude::*;
use super::model_arena::PyModelArena;
use super::module::module_ident_py::PyModuleIdent;

#[pymethods]
impl PyModelArena {
    // Register `module_i` as the model's top module (asserts none is set yet).
    fn set_top_module(&mut self, module_i: PyModuleIdent) {
        self.arena.set_top_module(module_i.into());
    }

    // Build the whole model starting from the top module's flow.
    fn build_flow(&mut self) {
        self.arena.build_flow();
    }
}
