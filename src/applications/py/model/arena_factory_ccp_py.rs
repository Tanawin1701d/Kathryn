// Python-facing complex-hardware (CCP) factories. Mirrors the host
// `arena_factory_ccp.rs`. All wrappers are user-declared (`is_user_com = true`).
// The arbiter `policy` is passed as its `kathryn.ArbSamePriPolicy` discriminant
// (declaration order, 0-based) — the int comes from Rust, so the two sides never
// disagree.

use pyo3::prelude::*;
use pyo3::exceptions::PyValueError;
use super::model_arena::PyModelArena;
use super::hw_component::common::hcp_ident_py::PyHcpIdent;
use super::hw_component::common::slice_py::PySlice;
use super::complex_hardware::ccp_ident_py::PyCcpIdent;
use crate::model::complex_hardware::arb::{ArbLockedChannel, ArbSamePriPolicy};

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

    // Add a leaf (its own req/ack wires) to `arb_i`; returns the leaf index.
    fn arb_add_leaf(&mut self, arb_i: PyCcpIdent, priority: i32) -> usize {
        self.arena.arb_add_leaf(arb_i.into(), priority)
    }

    // Add a leaf with one channel hard-tied to constant 1 to `arb_i`; `channel`
    // is a `kathryn.ArbLockedChannel` member. Returns the leaf index.
    fn arb_add_leaf_locked(&mut self, arb_i: PyCcpIdent, priority: i32, channel: u32) -> PyResult<usize> {
        let channel = ArbLockedChannel::from_index(channel)
            .ok_or_else(|| PyValueError::new_err(format!("ArbLockedChannel index out of range: {channel}")))?;
        Ok(self.arena.arb_add_leaf_locked(arb_i.into(), priority, channel))
    }

}
