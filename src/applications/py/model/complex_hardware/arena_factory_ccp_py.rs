// Python-facing complex-hardware (CCP) factories. Mirrors the host
// `arena_factory_ccp.rs`. All wrappers are user-declared (`is_user_com = true`).
// The arbiter `policy` is passed as its `kathryn.ArbSamePriPolicy` discriminant
// (declaration order, 0-based) — the int comes from Rust, so the two sides never
// disagree. Type-specific operations live in `arb/` and `karray/`.

use pyo3::prelude::*;
use pyo3::exceptions::PyValueError;
use super::super::model_arena::PyModelArena;
use super::ccp_ident_py::PyCcpIdent;
use crate::model::complex_hardware::arb::ArbSamePriPolicy;
use crate::model::complex_hardware::karray::KARRAY_BACKINGS;
use crate::model::hw_component::common::hcp_ident::HwComponentType;

#[pymethods]
impl PyModelArena {
    // ---- Arb ---------------------------------------------------------------

    // Declare a user arbiter (req/ack wires are 1 bit) with same-priority `policy`
    // (a `kathryn.ArbSamePriPolicy` member); returns its handle.
    fn mk_arb(&mut self, name: &str, policy: u32) -> PyResult<PyCcpIdent> {
        let policy = ArbSamePriPolicy::from_index(policy)
            .ok_or_else(|| PyValueError::new_err(format!("ArbSamePriPolicy index out of range: {policy}")))?;
        Ok(self.arena.make_arb(true, name, policy).into())
    }

    // ---- Karray ------------------------------------------------------------

    // Declare a user Karray of `shape` whose element is the record `fields`
    // (`(name, width)` pairs). `backing` is a `kathryn.HwComponentType` member —
    // must be Reg / Wire; returns the CCP handle. The backing hardware (one HCP
    // per element-field) is materialised up front.
    fn mk_karray(
        &mut self,
        name   : &str,
        shape  : Vec<usize>,
        fields : Vec<(String, i32)>,
        backing: usize,
    ) -> PyResult<PyCcpIdent> {
        let backing = HwComponentType::from_index(backing)
            .ok_or_else(|| PyValueError::new_err(format!("HwComponentType index out of range: {backing}")))?;
        if !KARRAY_BACKINGS.contains(&backing) {
            return Err(PyValueError::new_err(format!(
                "Karray backing must be HwComponentType.REG / WIRE, got {backing}")));
        }
        Ok(self.arena.make_karray(true, name, shape, fields, backing).into())
    }
}
