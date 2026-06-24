// Python-facing expression factories. `mk_expression` builds any binary op in a
// single function — Python passes the op as its `LogicOp` discriminant (the
// variant's declaration order, 0-based). `ExtendBit` takes a literal width
// instead of a second signal, so it stays its own `mk_extend_bit` method.
// Single-operand ops (`~`, `!`) go through `mk_expression_single`. `Assign` and
// `Dummy` are unsupported here; `make_expression` would panic.

use pyo3::prelude::*;
use pyo3::exceptions::PyValueError;
use super::super::model_arena::PyModelArena;
use super::common::hcp_ident_py::PyHcpIdent;
use super::common::slice_py::PySlice;
use super::common::operand_py::PyOperand;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::util::math::vary_val::VaryVal;
use num_bigint::BigInt;

#[pymethods]
impl PyModelArena {
    // Build a binary user expression `op(a, b)`; pass `op` as a `kathryn.LogicOp`
    // member (e.g. `LogicOp.ArithPlus`) — its int value comes from Rust, so the
    // two sides never disagree. Slices optional (default = full signal).
    #[pyo3(signature = (name, op, a, b, a_slice=None, b_slice=None))]
    fn mk_expression(&mut self, name: &str, op: u32, a: PyOperand, b: PyOperand, a_slice: Option<PySlice>, b_slice: Option<PySlice>) -> PyResult<PyHcpIdent> {
        let op = LogicOp::from_index(op).ok_or_else(|| PyValueError::new_err(format!("LogicOp index out of range: {op}")))?;
        if op.is_single_opr() || matches!(op, LogicOp::ExtendBit | LogicOp::Dummy) {
            return Err(PyValueError::new_err(format!("{op:?} is not a binary expression op (use mk_expression_single / mk_extend_bit / the assign path)")));
        }
        let (a_i, b_i) = self.resolve_binop_operands(name, a, b)?;
        Ok(self.arena.make_expression(true, name, op, a_i, b_i, a_slice.map(Into::into), b_slice.map(Into::into)).into())
    }

    // ---- single-operand (`~`, `!`) ------------------------------------------

    // Build a unary user expression `op(a)` — `BitwiseInvr` (`~`) or `LogicalNot`
    // (`!`). `Assign` is single-operand too but goes through gen_basic_assign.
    #[pyo3(signature = (name, op, a, a_slice=None))]
    fn mk_expression_single(&mut self, name: &str, op: u32, a: PyHcpIdent, a_slice: Option<PySlice>) -> PyResult<PyHcpIdent> {
        let op = LogicOp::from_index(op).ok_or_else(|| PyValueError::new_err(format!("LogicOp index out of range: {op}")))?;
        if !matches!(op, LogicOp::BitwiseInvr | LogicOp::LogicalNot) {
            return Err(PyValueError::new_err(format!("{op:?} is not a unary expression op (use mk_expression / the assign path)")));
        }
        Ok(self.arena.make_expression_single(true, name, op, a.into(), a_slice.map(Into::into)).into())
    }

    // ---- ExtendBit (const-operand) ------------------------------------------

    // Zero/bit-extend `a` to a literal width `c`; `a_slice` optional (full signal).
    #[pyo3(signature = (name, a, c, a_slice=None))]
    fn mk_extend_bit(&mut self, name: &str, a: PyHcpIdent, c: i32, a_slice: Option<PySlice>) -> PyHcpIdent {
        self.arena.make_expression_constant(true, name, LogicOp::ExtendBit, a.into(), c, a_slice.map(Into::into)).into()
    }
}

// ---- int-literal -> val helpers (shared with the assign path) ---------------
// Not #[pymethods]: plain inherent methods reused by arena_impl_hwc_py.rs.
impl PyModelArena {
    // Build a constant `val` of `bit_width`, sized to match the sibling operand.
    // `value` is arbitrary-precision, so any width/magnitude works. Masking to the
    // low `width` bits uses BigInt's two's-complement bitwise semantics, so a
    // negative literal sign-extends correctly across every limb. <=64-bit takes the
    // fast u64 path; wider widths go through limbs (VaryVal), as the `val` class does.
    pub(crate) fn make_const_val(&mut self, base_name: &str, value: &BigInt, bit_width: i32) -> HcpIdent {
        let width = bit_width.max(0) as usize;
        let name  = format!("{base_name}_c");
        if width == 0 {
            return self.arena.make_val(true, &name, bit_width, 0);
        }
        // `value & (2^width - 1)` keeps the low `width` bits as a non-negative int.
        let mask   = (BigInt::from(1) << width) - 1;
        let masked = (value & &mask).to_biguint().expect("masked value is non-negative");
        let limbs  = masked.to_u64_digits();                     // little-endian; [] for zero
        if width <= 64 {
            self.arena.make_val(true, &name, bit_width, limbs.first().copied().unwrap_or(0))
        } else {
            self.arena.make_val_vv(true, &name, bit_width, VaryVal::from_limbs(limbs, width))
        }
    }

    // Resolve a binop operand pair, wrapping an int into a val sized to the
    // sibling signal's width. Two int literals is an error (Python folds those).
    pub(crate) fn resolve_binop_operands(
        &mut self,
        name: &str,
        a   : PyOperand,
        b   : PyOperand,
    ) -> PyResult<(HcpIdent, HcpIdent)> {
        match (a, b) {
            (PyOperand::Ident(x), PyOperand::Ident(y)) => Ok((x.into(), y.into())),
            (PyOperand::Ident(x), PyOperand::Int(n))   => {
                let w = self.arena.get_hw_bit_sz(&x.into());
                Ok((x.into(), self.make_const_val(name, &n, w)))
            }
            (PyOperand::Int(n), PyOperand::Ident(y))   => {
                let w = self.arena.get_hw_bit_sz(&y.into());
                Ok((self.make_const_val(name, &n, w), y.into()))
            }
            (PyOperand::Int(_), PyOperand::Int(_)) =>
                Err(PyValueError::new_err("expression needs at least one signal operand, not two ints")),
        }
    }
}
