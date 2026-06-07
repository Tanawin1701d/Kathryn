// Python-facing owner of a single `ModelArena`. Single-threaded by design
// (`unsendable`); Python holds one of these and drives all construction
// through it, exactly mirroring how the arena is the sole Rust-side owner.

use pyo3::prelude::*;
use crate::model::model_arena::ModelArena;

#[pyclass(name = "ModelArena", unsendable)]
pub struct PyModelArena {
    pub(crate) arena: ModelArena,
}

#[pymethods]
impl PyModelArena {
    // Empty arena; the caller creates and registers the top module explicitly
    // via mk_module + set_top_module.
    #[new]
    fn new() -> Self {
        Self { arena: ModelArena::new() }
    }
}
