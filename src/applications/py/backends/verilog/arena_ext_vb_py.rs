// Python face of the Verilog backend's emitted-name queries (arena_ext_vb.rs).
// - Lives with the BACKEND connector, not on the idents: the emitted name is a
//   backend property (each emitter names its own way), and a future backend
//   (vhdl, chisel, ...) exposes its own `*_<backend>_name` pair here-alike.
// - Consumer: py/kathryn/sim_manifest.py (the sim-assist manifest writer).

use pyo3::prelude::*;
use super::super::super::model::model_arena::PyModelArena;
use super::super::super::model::hw_component::common::hcp_ident_py::PyHcpIdent;
use super::super::super::model::module::module_ident_py::PyModuleIdent;

#[pymethods]
impl PyModelArena {
    // The exact identifier the Verilog emitter writes for this HCP.
    fn hcp_verilog_name(&mut self, hcp_i: PyHcpIdent) -> String {
        self.arena.hcp_sim_name_vb(hcp_i.into())
    }

    // The exact Verilog module name — also the sub-module INSTANCE name.
    fn module_verilog_name(&mut self, module_i: PyModuleIdent) -> String {
        self.arena.module_sim_name_vb(module_i.into())
    }
}
