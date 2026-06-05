// Python-facing expression factories. `mk_expression` builds any binary op in a
// single function — Python passes the op as its `LogicOp` discriminant (the
// variant's declaration order, 0-based). `ExtendBit` takes a literal width
// instead of a second signal, so it stays its own `mk_extend_bit` method.
// `Assign` and `Dummy` are unsupported here; `make_expression` would panic.

use pyo3::prelude::*;
use pyo3::exceptions::PyValueError;
use super::model_arena::PyModelArena;
use super::hw_component::common::hcp_ident_py::PyHcpIdent;
use super::hw_component::common::slice_py::PySlice;
use crate::model::hw_component::common::operation::LogicOp;

#[pymethods]
impl PyModelArena {
    // Build a binary user expression `op(a, b)`; pass `op` as a `kathryn.LogicOp`
    // member (e.g. `LogicOp.ArithPlus`) — its int value comes from Rust, so the
    // two sides never disagree. Slices optional (default = full signal).
    #[pyo3(signature = (name, op, a, b, a_slice=None, b_slice=None))]
    fn mk_expression(&mut self, name: &str, op: u32, a: PyHcpIdent, b: PyHcpIdent, a_slice: Option<PySlice>, b_slice: Option<PySlice>) -> PyResult<PyHcpIdent> {
        let op = LogicOp::from_index(op).ok_or_else(|| PyValueError::new_err(format!("LogicOp index out of range: {op}")))?;
        if matches!(op, LogicOp::ExtendBit | LogicOp::Assign | LogicOp::Dummy) {
            return Err(PyValueError::new_err(format!("{op:?} is not a binary expression op (use mk_extend_bit / the assign path)")));
        }
        Ok(self.arena.make_expression(true, name, op, a.into(), b.into(), a_slice.map(Into::into), b_slice.map(Into::into)).into())
    }

    // ---- ExtendBit (const-operand) ------------------------------------------

    // Zero/bit-extend `a` to a literal width `c`; `a_slice` optional (full signal).
    #[pyo3(signature = (name, a, c, a_slice=None))]
    fn mk_extend_bit(&mut self, name: &str, a: PyHcpIdent, c: i32, a_slice: Option<PySlice>) -> PyHcpIdent {
        self.arena.make_expression_constant(true, name, LogicOp::ExtendBit, a.into(), c, a_slice.map(Into::into)).into()
    }
}
