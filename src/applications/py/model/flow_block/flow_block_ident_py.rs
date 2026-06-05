// Python-facing wrapper over the core `FlowBlockIdent`. The core type stays a
// pure Copy handle with no PyO3 macros; this newtype is the only Python-visible
// face. Every operation goes back through `PyModelArena`, never this object.

use pyo3::prelude::*;
use crate::model::common::identifier::Identifiable;
use crate::model::flow_block::FlowBlockIdent;

// Opaque handle to a flow block. Carries identity + block type only.
#[pyclass(name = "FlowBlockIdent", from_py_object)]
#[derive(Clone, Copy)]
pub struct PyFlowBlockIdent {
    pub(crate) inner: FlowBlockIdent,
}

#[pymethods]
impl PyFlowBlockIdent {
    // Process-wide unique id of the referenced block.
    #[getter]
    fn global_id(&self) -> u64 { self.inner.get_global_id() }

    // Block-type short tag, e.g. "SEQUENTIAL".
    #[getter]
    fn block_type(&self) -> String { self.inner.get_block_type().to_string() }

    fn __repr__(&self) -> String {
        format!("FlowBlockIdent(global_id={}, block_type={})",
                self.inner.get_global_id(),
                self.inner.get_block_type())
    }
}

impl From<FlowBlockIdent> for PyFlowBlockIdent {
    fn from(inner: FlowBlockIdent) -> Self { Self { inner } }
}

impl From<PyFlowBlockIdent> for FlowBlockIdent {
    fn from(py: PyFlowBlockIdent) -> Self { py.inner }
}
