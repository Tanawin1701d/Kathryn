// Python-facing wrapper over the core `HcpIdent`. The core type stays a pure
// Copy handle with no PyO3 macros; this newtype is the only Python-visible face.

use pyo3::prelude::*;
use crate::model::common::identifier::Identifiable;
use crate::model::hw_component::common::hcp_ident::HcpIdent;

// Opaque handle to a model component. Carries identity only — every operation
// goes back through `PyModelArena`, never through this object directly.
#[pyclass(name = "HcpIdent")]
#[derive(Clone, Copy)]
pub struct PyHcpIdent {
    pub(crate) inner: HcpIdent,
}

#[pymethods]
impl PyHcpIdent {
    // Process-wide unique id of the referenced component.
    #[getter]
    fn global_id(&self) -> u64 { self.inner.get_global_id() }

    // Hardware-type short prefix, e.g. "REG".
    #[getter]
    fn hw_type(&self) -> String { self.inner.get_hw_type().to_string() }

    fn __repr__(&self) -> String {
        format!("HcpIdent(global_id={}, hw_type={})",
                self.inner.get_global_id(),
                self.inner.get_hw_type())
    }
}

impl From<HcpIdent> for PyHcpIdent {
    fn from(inner: HcpIdent) -> Self { Self { inner } }
}

impl From<PyHcpIdent> for HcpIdent {
    fn from(py: PyHcpIdent) -> Self { py.inner }
}
