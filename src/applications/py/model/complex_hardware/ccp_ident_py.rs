// Python-facing wrapper over the core `CcpIdent`. The core type stays a pure
// Copy handle with no PyO3 macros; this newtype is the only Python-visible face.
// Every operation goes back through `PyModelArena`, never this object directly.

use pyo3::prelude::*;
use crate::model::common::identifier::Identifiable;
use crate::model::complex_hardware::common::ccp_ident::CcpIdent;

// Opaque handle to a complex component property (CCP). Carries identity + type only.
#[pyclass(name = "CcpIdent", from_py_object)]
#[derive(Clone, Copy)]
pub struct PyCcpIdent {
    pub(crate) inner: CcpIdent,
}

#[pymethods]
impl PyCcpIdent {
    // Process-wide unique id of the referenced CCP.
    #[getter]
    fn global_id(&self) -> u64 { self.inner.get_global_id() }

    // CCP-type short tag, e.g. "ARB".
    #[getter]
    fn ccp_type(&self) -> String { self.inner.get_ccp_type().to_string() }

    fn __repr__(&self) -> String {
        format!("CcpIdent(global_id={}, ccp_type={})",
                self.inner.get_global_id(),
                self.inner.get_ccp_type())
    }
}

impl From<CcpIdent> for PyCcpIdent {
    fn from(inner: CcpIdent) -> Self { Self { inner } }
}

impl From<PyCcpIdent> for CcpIdent {
    fn from(py: PyCcpIdent) -> Self { py.inner }
}
