// Python-facing module factories. Mirrors the host `arena_factory_module.rs`.
// `mk_module` builds any module (top or sub); the caller registers a top module
// via `set_top_module`. All wrappers are user-declared, so they go through the
// host `mk_*` (is_user_com = true) path.

use pyo3::prelude::*;
use super::super::model_arena::PyModelArena;
use super::module_ident_py::PyModuleIdent;

#[pymethods]
impl PyModelArena {
    // Declare a user sub-module. The parent is read off the module trace-stack
    // top (the module currently being constructed), so an enclosing module must
    // be open. Returns the new module's handle; open it with `initialize_module`
    // before declaring its components/flow blocks.
    fn mk_module(&mut self, name: &str) -> PyModuleIdent {
        self.arena.mk_module(name).into()
    }
}
