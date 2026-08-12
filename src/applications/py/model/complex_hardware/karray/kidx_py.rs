// The ONE place the Python-side Karray index encoding is decoded into the host
// `KIdx`, plus the host-outcome→Python bridging every Karray pymethod repeats
// (KarrayErr → the matching Python exception, skipped-field reports → warning).
//
// Per-dimension selector triple `(kind, ints, sigs)`:
//   ("static", [i],  [])      compile-time index
//   ("dyn",    [],   [sig])   runtime binary-encoded address signal
//   ("cus",    [],   bits)    WRITE side: one 1-bit enable per index (fn(i) output)
//   ("reduce", [],   [])      READ side: reduce fold; the select fn rides
//                             separately in the pymethod's `fns` list

use pyo3::prelude::*;
use pyo3::exceptions::{PyTypeError, PyValueError};
use super::super::super::hw_component::common::hcp_ident_py::PyHcpIdent;
use crate::model::complex_hardware::karray::{KarrayErr, KIdx};

pub(crate) type PyKIdx = (String, Vec<usize>, Vec<PyHcpIdent>);

// Decode the per-dimension selector triples into host `KIdx` values. Arity is
// validated here; bounds/rank/width are validated by the host resolution.
pub(crate) fn decode_kidx(sel: Vec<PyKIdx>) -> PyResult<Vec<KIdx>> {
    let mut out = Vec::with_capacity(sel.len());
    for (kind, ints, sigs) in sel {
        let kidx = match kind.as_str() {
            "static" if ints.len() == 1 && sigs.is_empty()  => KIdx::Static(ints[0]),
            "dyn"    if ints.is_empty() && sigs.len() == 1  => KIdx::Dyn(sigs[0].into()),
            "cus"    if ints.is_empty() && !sigs.is_empty() => KIdx::CusWe(sigs.into_iter().map(Into::into).collect()),
            "reduce" if ints.is_empty() && sigs.is_empty()  => KIdx::CusRd,
            other => return Err(PyValueError::new_err(
                format!("malformed Karray index selector of kind '{other}'"))),
        };
        out.push(kidx);
    }
    Ok(out)
}

// Engine errors surface through the env's associated error type; this lets the
// PyO3 env use `PyErr` directly (`E::Err: From<KarrayErr>`).
impl From<KarrayErr> for PyErr {
    fn from(e: KarrayErr) -> PyErr { karray_err_to_py(e) }
}

// Map a host `KarrayErr` to the matching Python exception: `Type` (operator /
// backing mismatch) -> TypeError, `Value` (rank / shape / bounds / field) ->
// ValueError. Use with `.map_err(karray_err_to_py)?`.
pub(crate) fn karray_err_to_py(e: KarrayErr) -> PyErr {
    match e {
        KarrayErr::Type (m) => PyTypeError ::new_err(m),
        KarrayErr::Value(m) => PyValueError::new_err(m),
    }
}

// Emit a Python `warnings.warn` for the field / source names an assign skipped
// (no-op when empty). The message is `"<context>: <skipped>"`.
pub(crate) fn warn_skipped_fields(py: Python<'_>, context: &str, skipped: &[String]) -> PyResult<()> {
    if !skipped.is_empty() {
        let msg = format!("{context}: {skipped:?}");
        py.import("warnings")?.call_method1("warn", (msg,))?;
    }
    Ok(())
}
