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
    // A top module is pushed up front so component factories always have a
    // module on the trace stack to attach to (make_reg panics otherwise).
    #[new]
    fn new(top_name: &str) -> Self {
        let mut arena = ModelArena::new();
        arena.mk_top_module(top_name);
        Self { arena }
    }
}
