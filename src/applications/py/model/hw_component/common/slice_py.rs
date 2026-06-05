// Python-facing wrapper over the core `Slice`. The core type stays a pure
// `Copy` value with no PyO3 macros; this newtype is the only Python-visible face.

use pyo3::prelude::*;
use crate::model::hw_component::common::slice::Slice;

// A bit range `[start, stop)`. Default `(-1, -1)` means the entire signal.
// `from_py_object` so it can be accepted by value as a factory argument.
#[pyclass(name = "Slice", from_py_object)]
#[derive(Clone, Copy)]
pub struct PySlice {
    pub(crate) inner: Slice,
}

#[pymethods]
impl PySlice {
    // Construct a slice; omit args for the full-signal default `(-1, -1)`.
    #[new]
    #[pyo3(signature = (start=-1, stop=-1))]
    fn new(start: i32, stop: i32) -> Self { Self { inner: Slice::new(start, stop) } }

    // Start index (inclusive).
    #[getter]
    fn start(&self) -> i32 { self.inner.start }

    // Stop index (exclusive).
    #[getter]
    fn stop(&self) -> i32 { self.inner.stop }

    // Number of bits the slice covers.
    #[getter]
    fn size(&self) -> i32 { self.inner.get_size() }

    // True when `start >= 0` and `start < stop`.
    fn is_valid(&self) -> bool { self.inner.check_valid_slice() }

    fn __repr__(&self) -> String {
        format!("Slice(start={}, stop={})", self.inner.start, self.inner.stop)
    }
}

impl From<Slice> for PySlice {
    fn from(inner: Slice) -> Self { Self { inner } }
}

impl From<PySlice> for Slice {
    fn from(py: PySlice) -> Self { py.inner }
}
