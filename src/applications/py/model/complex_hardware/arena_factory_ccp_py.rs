// Python-facing complex-hardware (CCP) factories. Mirrors the host
// `arena_factory_ccp.rs`. All wrappers are user-declared (`is_user_com = true`).
// The arbiter `policy` is passed as its `kathryn.ArbSamePriPolicy` discriminant
// (declaration order, 0-based) — the int comes from Rust, so the two sides never
// disagree.

use pyo3::prelude::*;
use pyo3::exceptions::{PyTypeError, PyValueError};
use super::super::model_arena::PyModelArena;
use super::super::hw_component::common::hcp_ident_py::PyHcpIdent;
use super::super::hw_component::common::slice_py::PySlice;
use super::ccp_ident_py::PyCcpIdent;
use crate::model::complex_hardware::arb::{ArbLockedChannel, ArbSamePriPolicy};
use crate::model::complex_hardware::karray::KARRAY_BACKINGS;
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HwComponentType};

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

    // ---- Karray ------------------------------------------------------------

    // Declare a user Karray of `shape` whose element is the packed record `fields`
    // (`(name, width)` pairs, first field at LSB). `backing` is a `kathryn.HwComponentType`
    // member — must be Reg / Wire / MemBlock; returns the CCP handle. The backing
    // hardware is materialised up front.
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
                "Karray backing must be HwComponentType.REG / WIRE / MEM_BLOCK, got {backing}")));
        }
        Ok(self.arena.make_karray(true, name, shape, fields, backing).into())
    }

    // Resolve a static index + field NAME to that field's own HCP (the `|=` / `*=`
    // target). `is_read` only matters for MemBlock backing (read vs write MemEle).
    // The Python handle keeps no field layout — the name is looked up here.
    fn karray_field_hcp(&mut self, karray_i: PyCcpIdent, indices: Vec<usize>, field: &str, is_read: bool) -> PyResult<PyHcpIdent> {
        let karray = self.arena.take_karray(karray_i.into());
        let out = match karray.field_index(field) {
            Some(idx) => Ok(karray.resolve_field_hcp(&indices, idx, is_read, &mut self.arena).into()),
            None      => Err(PyValueError::new_err(format!("Karray has no field '{field}'"))),
        };
        self.arena.replace_back_karray(karray);
        out
    }

    // Whole-element assignment: split the packed `src` across the per-field HCPs.
    // `expect_clocked` is the caller's operator intent — Some(true) for `|=`,
    // Some(false) for `*=` (mismatch with the backing raises TypeError), None for an
    // explicit `=` (no operator guard).
    #[pyo3(signature = (karray_i, indices, src_i, src_slice, expect_clocked=None))]
    fn karray_assign_element(
        &mut self,
        karray_i      : PyCcpIdent,
        indices       : Vec<usize>,
        src_i         : PyHcpIdent,
        src_slice     : PySlice,
        expect_clocked: Option<bool>,
    ) -> PyResult<()> {
        let karray = self.arena.take_karray(karray_i.into());
        let guard  = match expect_clocked {
            Some(true)  if !karray.is_clocked() => Err("`|=` (clocked assign) requires a reg- or memblock-backed Karray"),
            Some(false) if  karray.is_clocked() => Err("`*=` (combinational assign) requires a wire-backed Karray"),
            _                                   => Ok(()),
        };
        let out = match guard {
            Ok(())   => { karray.assign_element(&indices, src_i.into(), src_slice.into(), &mut self.arena); Ok(()) }
            Err(msg) => Err(PyTypeError::new_err(msg)),
        };
        self.arena.replace_back_karray(karray);
        out
    }

}
