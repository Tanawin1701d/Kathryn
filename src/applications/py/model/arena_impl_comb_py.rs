// Python-facing combinational combinators. Mirrors the host
// `arena_impl_comb.rs` — the topology and validation live THERE (any frontend
// gets them); this file only does what a connector is for: int-literal
// operands are wrapped into width-matched `val`s (BigInt path, shared
// `make_const_val`), the mux width is inferred while ints are still ints, and
// resize reports surface as Python warnings.

use pyo3::prelude::*;
use pyo3::exceptions::{PyTypeError, PyValueError};
use super::model_arena::PyModelArena;
use super::hw_component::arena_impl_hwc_py::warn_asm_resize;
use super::hw_component::common::hcp_ident_py::PyHcpIdent;
use super::hw_component::common::operand_py::PyOperand;
use super::hw_component::common::slice_py::PySlice;
use crate::model::common::identifier::Identifiable;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::slice::Slice;

#[pymethods]
impl PyModelArena {
    // `cond ? if_true : if_false` into a fresh combinational wire (returned).
    // Needs an open flow scope. `width` defaults to the first SIGNAL operand's
    // read width; an int arm is wrapped into a val of that width.
    #[pyo3(signature = (name, cond_i, cond_slice, if_true, if_false, true_slice=None, false_slice=None, width=None))]
    fn gen_mux(
        &mut self,
        py          : Python<'_>,
        name        : &str,
        cond_i      : PyHcpIdent,
        cond_slice  : PySlice,
        if_true     : PyOperand,
        if_false    : PyOperand,
        true_slice  : Option<PySlice>,
        false_slice : Option<PySlice>,
        width       : Option<i32>,
    ) -> PyResult<PyHcpIdent> {
        // Width of the first real signal among the arms — an int has none of
        // its own, so two int arms need an explicit width.
        let width = match width {
            Some(w) => w,
            None    => self.operand_read_width(&if_true, true_slice)
                .or_else(|| self.operand_read_width(&if_false, false_slice))
                .ok_or_else(|| PyTypeError::new_err(
                    "cannot infer a width from int operands alone — pass width=<bits> \
                     (an int literal is sized by whatever it is used with)"))?,
        };
        let (true_i,  true_slice)  = self.resolve_comb_operand(name, if_true,  true_slice,  width);
        let (false_i, false_slice) = self.resolve_comb_operand(name, if_false, false_slice, width);

        let (out_i, resizes) = self.arena.gen_mux(
                name, cond_i.into(), Some(cond_slice.into()),
                true_i, true_slice, false_i, false_slice, Some(width))
            .map_err(PyValueError::new_err)?;
        // Surface each arm's implicit width adjustment, as a direct `*=` would.
        for resize in resizes {
            warn_asm_resize(py, resize, out_i.get_global_name())?;
        }
        Ok(out_i.into())
    }

    // `sig` rotated left by the constant `amount`, as a pure expression.
    // `None` = a full turn (the identity) — the DSL returns the input ref
    // unchanged, slice view and all.
    #[pyo3(signature = (name, sig_i, sig_slice, amount, width=None))]
    fn gen_rotate_left(
        &mut self,
        name      : &str,
        sig_i     : PyHcpIdent,
        sig_slice : PySlice,
        amount    : i64,
        width     : Option<i32>,
    ) -> PyResult<Option<PyHcpIdent>> {
        self.arena.gen_rotate_left(name, sig_i.into(), Some(sig_slice.into()), amount, width)
            .map(|rot_i| rot_i.map(Into::into))
            .map_err(PyValueError::new_err)
    }

    // Balanced OR reduce over `terms`. Empty -> a constant-0 val; a single
    // term -> `None` (the DSL keeps using that term unchanged).
    fn gen_any_of(
        &mut self,
        name  : &str,
        terms : Vec<(PyHcpIdent, PySlice)>,
    ) -> Option<PyHcpIdent> {
        let terms = terms.into_iter().map(|(term_i, s)| (term_i.into(), s.into())).collect();
        self.arena.gen_any_of(name, terms).map(Into::into)
    }

    // Balanced adder tree counting the set bits of `bits`; the default width
    // cannot overflow. Empty is an error (a sum over nothing has no width).
    #[pyo3(signature = (name, bits, width=None))]
    fn gen_sum_cnt(
        &mut self,
        name  : &str,
        bits  : Vec<(PyHcpIdent, PySlice)>,
        width : Option<i32>,
    ) -> PyResult<PyHcpIdent> {
        let bits = bits.into_iter().map(|(bit_i, s)| (bit_i.into(), s.into())).collect();
        self.arena.gen_sum_cnt(name, bits, width)
            .map(Into::into)
            .map_err(PyValueError::new_err)
    }
}

// ---- internal helpers (not exposed to Python) ----

impl PyModelArena {
    // Read width of a signal operand (its slice, else its full width); an int
    // literal has none — the caller falls through to the sibling operand.
    fn operand_read_width(&self, operand: &PyOperand, slice: Option<PySlice>) -> Option<i32> {
        match operand {
            PyOperand::Ident(x) => Some(match slice {
                Some(s) => Slice::from(s).get_size(),
                None    => self.arena.get_hw_bit_sz(&(*x).into()),
            }),
            PyOperand::Int(_)   => None,
        }
    }

    // Resolve one mux arm: a signal passes through with its slice; an int is
    // wrapped into a val of the mux width (full-slice, like the assign path).
    fn resolve_comb_operand(
        &mut self,
        name    : &str,
        operand : PyOperand,
        slice   : Option<PySlice>,
        width   : i32,
    ) -> (HcpIdent, Option<Slice>) {
        match operand {
            PyOperand::Ident(x) => (x.into(), slice.map(Into::into)),
            PyOperand::Int(n)   => (self.make_const_val(name, &n, width), Some(Slice::new(0, width))),
        }
    }
}
