// Python-facing Arb operations. Mirrors the host `arb/arena_impl_ccp_arp.rs`
// (configure + wire accessors on an already-created Arb). The factory `mk_arb`
// lives in the shared `arena_factory_ccp_py.rs`.

use pyo3::prelude::*;
use pyo3::exceptions::PyValueError;
use super::super::super::model_arena::PyModelArena;
use super::super::super::hw_component::common::hcp_ident_py::PyHcpIdent;
use super::super::super::hw_component::common::slice_py::PySlice;
use super::super::ccp_ident_py::PyCcpIdent;
use crate::model::complex_hardware::arb::ArbLockedChannel;
use crate::model::hw_component::common::hcp_ident::HcpIdent;

#[pymethods]
impl PyModelArena {
    // ---- leaves ------------------------------------------------------------

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

    // ---- control-signal binders --------------------------------------------
    // Each gate signal must resolve to exactly one bit; `check_cond_slice_match`
    // raises a ValueError at construction instead of letting the host panic on
    // its 1-bit assert during build.

    // Bind the single master-ack gate source (optionally a bit-slice of it).
    #[pyo3(signature = (arb_i, src_i, src_slice=None))]
    fn arb_set_master_ack(&mut self, arb_i: PyCcpIdent, src_i: PyHcpIdent, src_slice: Option<PySlice>) -> PyResult<()> {
        let src_i: HcpIdent = src_i.into();
        self.check_cond_slice_match(src_i, src_slice)?;
        self.arena.arb_set_master_ack(arb_i.into(), src_i, src_slice.map(Into::into));
        Ok(())
    }

    // Bind the optional hold signal (freezes every grant while asserted).
    #[pyo3(signature = (arb_i, cond_i, cond_slice=None))]
    fn arb_set_hold(&mut self, arb_i: PyCcpIdent, cond_i: PyHcpIdent, cond_slice: Option<PySlice>) -> PyResult<()> {
        let cond_i: HcpIdent = cond_i.into();
        self.check_cond_slice_match(cond_i, cond_slice)?;
        self.arena.arb_set_user_hold(arb_i.into(), cond_i, cond_slice.map(Into::into));
        Ok(())
    }

    // Bind the optional reset signal (clears every grant while asserted).
    #[pyo3(signature = (arb_i, cond_i, cond_slice=None))]
    fn arb_set_reset(&mut self, arb_i: PyCcpIdent, cond_i: PyHcpIdent, cond_slice: Option<PySlice>) -> PyResult<()> {
        let cond_i: HcpIdent = cond_i.into();
        self.check_cond_slice_match(cond_i, cond_slice)?;
        self.arena.arb_set_user_reset(arb_i.into(), cond_i, cond_slice.map(Into::into));
        Ok(())
    }

    // ---- wire accessors ----------------------------------------------------

    // Number of leaves registered on `arb_i`.
    fn arb_leaf_count(&mut self, arb_i: PyCcpIdent) -> usize {
        self.arena.arb_leaf_count(arb_i.into())
    }

    // The master-req wire (OR of every leaf request, 1 bit).
    fn arb_get_master_req_wire(&mut self, arb_i: PyCcpIdent) -> PyHcpIdent {
        self.arena.arb_get_master_req_wire_i(arb_i.into()).into()
    }

    // The request wire of leaf `idx` (driven by the user to contend).
    fn arb_get_leaf_req_wire(&mut self, arb_i: PyCcpIdent, idx: usize) -> PyHcpIdent {
        self.arena.arb_get_leaf_req_wire_i(arb_i.into(), idx).into()
    }

    // The ack wire of leaf `idx` (the grant, driven by the build pass).
    fn arb_get_leaf_ack_wire(&mut self, arb_i: PyCcpIdent, idx: usize) -> PyHcpIdent {
        self.arena.arb_get_leaf_ack_wire_i(arb_i.into(), idx).into()
    }
}
