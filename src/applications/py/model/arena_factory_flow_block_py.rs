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
use crate::model::hw_component::common::slice::Slice;

impl PyModelArena {
    // Validate the condition the block will gate on. The slice (when given) must
    // fit inside `cond_i`'s bit-width, and the *effective* condition — the slice
    // size, or the whole variable when no real range is given — must be exactly
    // 1 bit. Both mismatches are raised here as ValueError so they surface at
    // construction rather than as a host panic during build.
    fn check_cond_slice_match(&self, cond_i: HcpIdent, cond_slice: Option<PySlice>) -> PyResult<()> {
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

        // A flow-block condition must be a single bit.
        if cond_width != 1 {
            return Err(PyValueError::new_err(format!(
                "cond must be 1-bit, got {cond_width} (slice the cond variable down to one bit)")));
        }
        Ok(())
    }
}

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
    fn mk_flow_block_pip(&mut self, name: &str, arb_i: PyCcpIdent) -> PyFlowBlockIdent {
        self.arena.make_flow_block_pip(name, arb_i.into()).into()
    }

    // Pipeline block in auto-restart mode: the arb user-reset re-launches the flow.
    fn mk_flow_block_pip_auto_restart(&mut self, name: &str, arb_i: PyCcpIdent) -> PyFlowBlockIdent {
        self.arena.make_flow_block_pip_auto_restart(name, arb_i.into()).into()
    }
}
