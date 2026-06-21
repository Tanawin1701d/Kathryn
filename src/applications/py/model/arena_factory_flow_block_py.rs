// Python-facing flow-block factories. Mirrors the host
// `arena_factory_flow_block.rs` — every `make_flow_block_*` constructor is
// exposed here as `mk_flow_block_*`, returning the new block's handle. The
// host has no is_user_com distinction for flow blocks, so these are 1:1.

use pyo3::prelude::*;
use pyo3::exceptions::PyValueError;
use super::model_arena::PyModelArena;
use super::flow_block::flow_block_ident_py::PyFlowBlockIdent;
use super::hw_component::common::hcp_ident_py::PyHcpIdent;
use super::hw_component::common::slice_py::PySlice;
use super::complex_hardware::ccp_ident_py::PyCcpIdent;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::update_event::DEFAULT_UE_PRI_USER;

#[pymethods]
impl PyModelArena {
    // ---- seq ----------------------------------------------------------------

    // Sequential block: children run one after another.
    fn mk_flow_block_seq(&mut self, name: &str) -> PyFlowBlockIdent {
        self.arena.make_flow_block_seq(name).into()
    }

    // ---- par ----------------------------------------------------------------

    // Parallel block with auto-sync at the join.
    fn mk_flow_block_par_auto(&mut self, name: &str) -> PyFlowBlockIdent {
        self.arena.make_flow_block_par_auto(name).into()
    }

    // Parallel block with no join synchronisation.
    fn mk_flow_block_par_no_sync(&mut self, name: &str) -> PyFlowBlockIdent {
        self.arena.make_flow_block_par_no_sync(name).into()
    }

    // ---- cond: CIF / SIF ----------------------------------------------------

    // Combinational `if` gated by `cond_i[cond_slice]` (slice optional = whole signal).
    #[pyo3(signature = (name, cond_i, cond_slice=None))]
    fn mk_flow_block_cif(&mut self, name: &str, cond_i: PyHcpIdent, cond_slice: Option<PySlice>) -> PyResult<PyFlowBlockIdent> {
        let cond_i: HcpIdent = cond_i.into();
        self.check_cond_slice_match(cond_i, cond_slice)?;
        Ok(self.arena.make_flow_block_cif(name, cond_i, cond_slice.map(Into::into)).into())
    }

    // Sequential `if` gated by `cond_i[cond_slice]`.
    #[pyo3(signature = (name, cond_i, cond_slice=None))]
    fn mk_flow_block_sif(&mut self, name: &str, cond_i: PyHcpIdent, cond_slice: Option<PySlice>) -> PyResult<PyFlowBlockIdent> {
        let cond_i: HcpIdent = cond_i.into();
        self.check_cond_slice_match(cond_i, cond_slice)?;
        Ok(self.arena.make_flow_block_sif(name, cond_i, cond_slice.map(Into::into)).into())
    }

    // ---- cond_elif: CSELIF / CSELSE -----------------------------------------

    // `elif` continuation gated by `cond_i[cond_slice]`.
    #[pyo3(signature = (name, cond_i, cond_slice=None))]
    fn mk_flow_block_cselif(&mut self, name: &str, cond_i: PyHcpIdent, cond_slice: Option<PySlice>) -> PyResult<PyFlowBlockIdent> {
        let cond_i: HcpIdent = cond_i.into();
        self.check_cond_slice_match(cond_i, cond_slice)?;
        Ok(self.arena.make_flow_block_cselif(name, cond_i, cond_slice.map(Into::into)).into())
    }

    // `else` continuation (no condition).
    fn mk_flow_block_cselse(&mut self, name: &str) -> PyFlowBlockIdent {
        self.arena.make_flow_block_cselse(name).into()
    }

    // ---- zero_cond: ZIF / ZELIF / ZELSE -------------------------------------

    // Zero-cycle `if` gated by `cond_i[cond_slice]`.
    #[pyo3(signature = (name, cond_i, cond_slice=None))]
    fn mk_flow_block_zif(&mut self, name: &str, cond_i: PyHcpIdent, cond_slice: Option<PySlice>) -> PyResult<PyFlowBlockIdent> {
        let cond_i: HcpIdent = cond_i.into();
        self.check_cond_slice_match(cond_i, cond_slice)?;
        Ok(self.arena.make_flow_block_zif(name, cond_i, cond_slice.map(Into::into)).into())
    }

    // Zero-cycle `elif` continuation gated by `cond_i[cond_slice]`.
    #[pyo3(signature = (name, cond_i, cond_slice=None))]
    fn mk_flow_block_zelif(&mut self, name: &str, cond_i: PyHcpIdent, cond_slice: Option<PySlice>) -> PyResult<PyFlowBlockIdent> {
        let cond_i: HcpIdent = cond_i.into();
        self.check_cond_slice_match(cond_i, cond_slice)?;
        Ok(self.arena.make_flow_block_zelif(name, cond_i, cond_slice.map(Into::into)).into())
    }

    // Zero-cycle `else` continuation (no condition).
    fn mk_flow_block_zelse(&mut self, name: &str) -> PyFlowBlockIdent {
        self.arena.make_flow_block_zelse(name).into()
    }

    // ---- zero_switch: ZSTATE / ZCASE ----------------------------------------

    // Zero-cycle switch over the `state_i` variable.
    fn mk_flow_block_zstate(&mut self, name: &str, state_i: PyHcpIdent) -> PyFlowBlockIdent {
        self.arena.make_flow_block_zstate(name, state_i.into()).into()
    }

    // One `case` arm of a zero-cycle switch, selected when state == `match_val`.
    fn mk_flow_block_zcase(&mut self, name: &str, match_val: i32) -> PyFlowBlockIdent {
        self.arena.make_flow_block_zcase(name, match_val).into()
    }

    // ---- while: CWHILE / SWHILE ---------------------------------------------

    // Combinational `while` gated by `cond_i[cond_slice]`.
    #[pyo3(signature = (name, cond_i, cond_slice=None))]
    fn mk_flow_block_cwhile(&mut self, name: &str, cond_i: PyHcpIdent, cond_slice: Option<PySlice>) -> PyResult<PyFlowBlockIdent> {
        let cond_i: HcpIdent = cond_i.into();
        self.check_cond_slice_match(cond_i, cond_slice)?;
        Ok(self.arena.make_flow_block_cwhile(name, cond_i, cond_slice.map(Into::into)).into())
    }

    // Sequential `while` gated by `cond_i[cond_slice]`.
    #[pyo3(signature = (name, cond_i, cond_slice=None))]
    fn mk_flow_block_swhile(&mut self, name: &str, cond_i: PyHcpIdent, cond_slice: Option<PySlice>) -> PyResult<PyFlowBlockIdent> {
        let cond_i: HcpIdent = cond_i.into();
        self.check_cond_slice_match(cond_i, cond_slice)?;
        Ok(self.arena.make_flow_block_swhile(name, cond_i, cond_slice.map(Into::into)).into())
    }

    // ---- do_while -----------------------------------------------------------

    // `do { } while(cond_i[cond_slice])` — body runs at least once.
    #[pyo3(signature = (name, cond_i, cond_slice=None))]
    fn mk_flow_block_do_while(&mut self, name: &str, cond_i: PyHcpIdent, cond_slice: Option<PySlice>) -> PyResult<PyFlowBlockIdent> {
        let cond_i: HcpIdent = cond_i.into();
        self.check_cond_slice_match(cond_i, cond_slice)?;
        Ok(self.arena.make_flow_block_do_while(name, cond_i, cond_slice.map(Into::into)).into())
    }

    // ---- counter_loop -------------------------------------------------------

    // Fixed-count loop running until `last_loop_cnt`.
    fn mk_flow_block_counter_loop(&mut self, name: &str, last_loop_cnt: i32) -> PyFlowBlockIdent {
        self.arena.make_flow_block_counter_loop(name, last_loop_cnt).into()
    }

    // ---- wait: SCWAIT / SYWAIT ----------------------------------------------

    // Stall until `cond_i[cond_slice]` fires. The condition must resolve to one
    // bit (same check as the cond blocks); the slice is handled natively by the
    // wait register, not via a SliceBit expression.
    #[pyo3(signature = (name, cond_i, cond_slice=None))]
    fn mk_flow_block_scwait(&mut self, name: &str, cond_i: PyHcpIdent, cond_slice: Option<PySlice>) -> PyResult<PyFlowBlockIdent> {
        let cond_i: HcpIdent = cond_i.into();
        self.check_cond_slice_match(cond_i, cond_slice)?;
        Ok(self.arena.make_flow_block_scwait(name, cond_i, cond_slice.map(Into::into)).into())
    }

    // Stall for a fixed `cycle` clock count.
    fn mk_flow_block_sywait(&mut self, name: &str, cycle: i32) -> PyResult<PyFlowBlockIdent> {
        if cycle <= 0 {
            return Err(PyValueError::new_err(format!("sywait cycle must be positive, got {cycle}")));
        }
        Ok(self.arena.make_flow_block_sywait(name, cycle).into())
    }

    // ---- pip: pipeline ------------------------------------------------------

    // Pipeline block gated by arbiter `arb_i`; holds exactly one body sub-block.
    // The pip's leaf is added at `priority` (default user priority); `auto_req`
    // (default false) keeps a normal leaf, pass true to Req-lock it (always requesting).
    // `auto_restart` (default false) routes the arb user-reset back into the start.
    #[pyo3(signature = (name, arb_i, priority=None, auto_req=false, auto_restart=false))]
    fn mk_flow_block_pip(&mut self, name: &str, arb_i: PyCcpIdent, priority: Option<i32>, auto_req: bool, auto_restart: bool) -> PyFlowBlockIdent {
        let priority = priority.unwrap_or(DEFAULT_UE_PRI_USER);
        self.arena.make_flow_block_pip(name, arb_i.into(), priority, auto_req, auto_restart).into()
    }

    // ---- zync ---------------------------------------------------------------

    // Zync block contending on arbiter `arb_i`; its channel (leaf) is allocated
    // here at creation time. `priority` is optional and defaults to the user-default
    // UE priority when omitted. `auto_ack` (default false) keeps a normal leaf that
    // waits for the arbiter grant; pass true to Ack-lock it (always granted).
    #[pyo3(signature = (name, arb_i, priority=None, auto_ack=false))]
    fn mk_flow_block_zync(&mut self, name: &str, arb_i: PyCcpIdent, priority: Option<i32>, auto_ack: bool) -> PyFlowBlockIdent {
        let priority = priority.unwrap_or(DEFAULT_UE_PRI_USER);
        self.arena.make_flow_block_zync(name, arb_i.into(), priority, auto_ack).into()
    }
}
