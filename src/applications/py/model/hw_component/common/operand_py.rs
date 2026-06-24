// A Python-side expression/assignment operand: either an existing signal handle
// or a raw int literal that the connector wraps into a width-matched `val`.

use pyo3::prelude::*;
use num_bigint::BigInt;
use super::hcp_ident_py::PyHcpIdent;

// FromPyObject tries variants top-to-bottom: a PyHcpIdent (a signal) resolves
// first; a Python int never extracts as a pyclass, so literals fall to `Int`.
// `BigInt` carries the literal at arbitrary precision (Python ints are unbounded),
// so a constant of any width crosses the boundary without truncation.
#[derive(FromPyObject)]
pub(crate) enum PyOperand {
    Ident(PyHcpIdent),   // an existing signal
    Int  (BigInt),       // a literal of any magnitude / sign
}
