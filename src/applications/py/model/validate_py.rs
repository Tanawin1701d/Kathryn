// Shared input validators on `PyModelArena`. These are plain (non-`#[pymethods]`)
// helpers reused across the factory files so a single rule has one definition —
// e.g. the "condition must be 1 bit" check shared by the conditional flow blocks
// and the arbiter gate-signal binders. Validation failures surface as a Python
// `ValueError` at construction rather than a host panic during build.

use pyo3::prelude::*;
use pyo3::exceptions::PyValueError;
use super::model_arena::PyModelArena;
use super::hw_component::common::slice_py::PySlice;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::slice::Slice;

impl PyModelArena {
    // Validate a single-bit condition/gate signal. The slice (when given) must
    // fit inside `cond_i`'s bit-width, and the *effective* condition — the slice
    // size, or the whole variable when no real range is given — must be exactly
    // 1 bit.
    pub(super) fn check_cond_slice_match(&self, cond_i: HcpIdent, cond_slice: Option<PySlice>) -> PyResult<()> {
        let var_width = self.arena.get_hw_bit_sz(&cond_i);

        // Effective condition width: the slice when a real range is given,
        // otherwise the whole variable (default/invalid slice = whole signal).
        let cond_width = match cond_slice {
            Some(s) => {
                let s: Slice = s.into();
                if !s.check_valid_slice() {
                    var_width
                } else {
                    if s.stop > var_width {
                        return Err(PyValueError::new_err(format!(
                            "cond slice {s:?} does not fit cond variable of width {var_width}")));
                    }
                    s.get_size()
                }
            }
            None => var_width,
        };

        // A flow-block condition / arb gate must be a single bit.
        if cond_width != 1 {
            return Err(PyValueError::new_err(format!(
                "cond must be 1-bit, got {cond_width} (slice the cond variable down to one bit)")));
        }
        Ok(())
    }
}
