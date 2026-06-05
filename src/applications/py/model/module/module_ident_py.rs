// Python-facing wrapper over the core `ModuleIdent`. The core type stays a pure
// Copy handle with no PyO3 macros; this newtype is the only Python-visible face.
// Every operation goes back through `PyModelArena`, never this object directly.

use pyo3::prelude::*;
use crate::model::common::identifier::Identifiable;
use crate::model::module::module_ident::ModuleIdent;

// Opaque handle to a module. Carries identity + nesting depth only.
#[pyclass(name = "ModuleIdent", from_py_object)]
#[derive(Clone, Copy)]
pub struct PyModuleIdent {
    pub(crate) inner: ModuleIdent,
}

#[pymethods]
impl PyModuleIdent {
    // Process-wide unique id of the referenced module.
    #[getter]
    fn global_id(&self) -> u64 { self.inner.get_global_id() }

    // Nesting depth: 0 = top module, +1 per sub-module level.
    #[getter]
    fn depth_level(&self) -> u32 { self.inner.get_depth_level() }

    fn __repr__(&self) -> String {
        format!("ModuleIdent(global_id={}, depth_level={})",
                self.inner.get_global_id(),
                self.inner.get_depth_level())
    }
}

impl From<ModuleIdent> for PyModuleIdent {
    fn from(inner: ModuleIdent) -> Self { Self { inner } }
}

impl From<PyModuleIdent> for ModuleIdent {
    fn from(py: PyModuleIdent) -> Self { py.inner }
}
