// Python-facing owner of a single `ModelArena`. Single-threaded by design
// (`unsendable`); Python holds one of these and drives all construction
// through it, exactly mirroring how the arena is the sole Rust-side owner.

use pyo3::prelude::*;
use crate::model::model_arena::{ModelArena, ModuleInitStage};

#[pyclass(name = "ModelArena", unsendable)]
pub struct PyModelArena {
    pub(crate) arena: ModelArena,
}

#[pymethods]
impl PyModelArena {
    // A top module is pushed up front so component factories always have a
    // module on the trace stack to attach to (make_reg panics otherwise).
    // `mk_top_module` pushes at CompInit; re-stage it to FlowBlockInit so it
    // matches the stage `initialize_module` opens sub-modules at and the stage
    // `finalize_flow_block` requires when attaching a top-level flow block.
    #[new]
    fn new(top_name: &str) -> Self {
        let mut arena = ModelArena::new();
        let top_i = arena.mk_top_module(top_name);
        arena.pop_module_trace_stack();
        arena.push_module_trace_stack(top_i, ModuleInitStage::FlowBlockInit);
        Self { arena }
    }
}
