// Shared Karray connector helpers — the one home for the host-outcome→Python
// bridging that every Karray pymethod repeats: mapping a `KarrayAsmErr` to the
// matching Python exception, and surfacing skipped-field reports as a warning.

use pyo3::prelude::*;
use pyo3::exceptions::{PyTypeError, PyValueError};
use crate::model::complex_hardware::karray::KarrayAsmErr;

// Map a host `KarrayAsmErr` to the matching Python exception: `Type` (operator /
// backing mismatch) -> TypeError, `Value` (shape / bounds / no-match) -> ValueError.
// Use with `.map_err(karray_err_to_py)?` on any Result the arena hands back.
pub(crate) fn karray_err_to_py(e: KarrayAsmErr) -> PyErr {
    match e {
        KarrayAsmErr::Type (m) => PyTypeError ::new_err(m),
        KarrayAsmErr::Value(m) => PyValueError::new_err(m),
    }
}

// Emit a Python `warnings.warn` for the field / source names an assign skipped
// (no-op when empty). `context` names the operation and the skip reason; the
// message is `"<context>: <skipped>"`.
pub(crate) fn warn_skipped_fields(py: Python<'_>, context: &str, skipped: &[String]) -> PyResult<()> {
    if !skipped.is_empty() {
        let msg = format!("{context}: {skipped:?}");
        py.import("warnings")?.call_method1("warn", (msg,))?;
    }
    Ok(())
}
