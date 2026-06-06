// Python-facing flow-block factories. Mirrors the host
// `arena_factory_flow_block.rs` — every `make_flow_block_*` constructor is
// exposed here as `mk_flow_block_*`, returning the new block's handle. The
// host has no is_user_com distinction for flow blocks, so these are 1:1.

use pyo3::prelude::*;
use super::model_arena::PyModelArena;
use super::flow_block::flow_block_ident_py::PyFlowBlockIdent;
use super::hw_component::common::hcp_ident_py::PyHcpIdent;

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

    // Combinational `if` gated by `cond_i`.
    fn mk_flow_block_cif(&mut self, name: &str, cond_i: PyHcpIdent) -> PyFlowBlockIdent {
        self.arena.make_flow_block_cif(name, cond_i.into()).into()
    }

    // Sequential `if` gated by `cond_i`.
    fn mk_flow_block_sif(&mut self, name: &str, cond_i: PyHcpIdent) -> PyFlowBlockIdent {
        self.arena.make_flow_block_sif(name, cond_i.into()).into()
    }

    // ---- cond_elif: CSELIF / CSELSE -----------------------------------------

    // `elif` continuation gated by `cond_i`.
    fn mk_flow_block_cselif(&mut self, name: &str, cond_i: PyHcpIdent) -> PyFlowBlockIdent {
        self.arena.make_flow_block_cselif(name, cond_i.into()).into()
    }

    // `else` continuation (no condition).
    fn mk_flow_block_cselse(&mut self, name: &str) -> PyFlowBlockIdent {
        self.arena.make_flow_block_cselse(name).into()
    }

    // ---- zero_cond: ZIF / ZELIF / ZELSE -------------------------------------

    // Zero-cycle `if` gated by `cond_i`.
    fn mk_flow_block_zif(&mut self, name: &str, cond_i: PyHcpIdent) -> PyFlowBlockIdent {
        self.arena.make_flow_block_zif(name, cond_i.into()).into()
    }

    // Zero-cycle `elif` continuation gated by `cond_i`.
    fn mk_flow_block_zelif(&mut self, name: &str, cond_i: PyHcpIdent) -> PyFlowBlockIdent {
        self.arena.make_flow_block_zelif(name, cond_i.into()).into()
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

    // Combinational `while` gated by `cond_i`.
    fn mk_flow_block_cwhile(&mut self, name: &str, cond_i: PyHcpIdent) -> PyFlowBlockIdent {
        self.arena.make_flow_block_cwhile(name, cond_i.into()).into()
    }

    // Sequential `while` gated by `cond_i`.
    fn mk_flow_block_swhile(&mut self, name: &str, cond_i: PyHcpIdent) -> PyFlowBlockIdent {
        self.arena.make_flow_block_swhile(name, cond_i.into()).into()
    }

    // ---- do_while -----------------------------------------------------------

    // `do { } while(cond_i)` — body runs at least once.
    fn mk_flow_block_do_while(&mut self, name: &str, cond_i: PyHcpIdent) -> PyFlowBlockIdent {
        self.arena.make_flow_block_do_while(name, cond_i.into()).into()
    }

    // ---- counter_loop -------------------------------------------------------

    // Fixed-count loop running until `last_loop_cnt`.
    fn mk_flow_block_counter_loop(&mut self, name: &str, last_loop_cnt: i32) -> PyFlowBlockIdent {
        self.arena.make_flow_block_counter_loop(name, last_loop_cnt).into()
    }
}
