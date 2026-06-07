// Python-facing wrapper over the Verilog backend. Mirrors the host
// `src/backends/verilog/backend.rs`. The backend is the arena's final consumer,
// so construction moves the arena out of the `PyModelArena`, leaving it empty.

use pyo3::prelude::*;
use crate::applications::py::model::model_arena::PyModelArena;
use crate::backends::verilog::backend::BackendVerilog;
use crate::model::model_arena::ModelArena;

#[pyclass(name = "BackendVerilog", unsendable)]
pub struct PyBackendVerilog {
    inner : BackendVerilog,
}

#[pymethods]
impl PyBackendVerilog {
    // Takes ownership of the arena — the source `PyModelArena` is left empty.
    #[new]
    fn new(arena: &mut PyModelArena) -> Self {
        let model_arena = std::mem::replace(&mut arena.arena, ModelArena::new());
        Self { inner: BackendVerilog::new(model_arena) }
    }

    // Run all three backend phases; output_dir must already exist.
    fn emit(&mut self, output_dir: &str) {
        self.inner.emit(output_dir);
    }
}
