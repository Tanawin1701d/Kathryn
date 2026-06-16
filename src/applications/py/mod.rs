// PyO3 binding root. Mirrors the host module tree under `py/` and exposes the
// Python module entry point. Every Python macro in the crate lives under here.

use pyo3::prelude::*;
use pyo3::types::PyDict;

pub mod model;
pub mod backends;

use model::model_arena::PyModelArena;
use backends::verilog::backend_py::PyBackendVerilog;
use model::hw_component::common::hcp_ident_py::PyHcpIdent;
use model::hw_component::common::slice_py::PySlice;
use model::complex_hardware::ccp_ident_py::PyCcpIdent;
use model::flow_block::flow_block_ident_py::PyFlowBlockIdent;
use model::module::module_ident_py::PyModuleIdent;
use model::controller::asm_mode_py::add_asm_priority_consts;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::complex_hardware::arb::ArbSamePriPolicy;
use crate::model::flow_block::FlowBlockType;

// Build `kathryn.LogicOp` as a Python IntEnum sourced entirely from the core
// `LogicOp` via `from_index` / `variant_name`. Python never hardcodes op ints,
// and `mk_expression` decodes through the *same* `from_index`, so the two sides
// share one source of truth and cannot misalign.
fn add_logic_op_enum(m: &Bound<'_, PyModule>) -> PyResult<()> {
    let py      = m.py();
    let members = PyDict::new(py);
    let mut idx = 0u32;
    // Walk every LogicOp by index until from_index runs out, mirroring each
    // variant name → int into the dict that backs the Python IntEnum.
    while let Some(op) = LogicOp::from_index(idx) {
        members.set_item(op.variant_name(), idx)?;  // enum member: name = idx
        idx += 1;
    }
    let int_enum = py.import("enum")?.getattr("IntEnum")?;
    m.add("LogicOp", int_enum.call1(("LogicOp", &members))?)?;
    Ok(())
}

// Build `kathryn.ArbSamePriPolicy` as a Python IntEnum sourced entirely from the
// core `ArbSamePriPolicy` via `from_index` / `variant_name`. `mk_arb` decodes the
// int back through the *same* `from_index`, so the two sides cannot misalign.
fn add_arb_same_pri_policy_enum(m: &Bound<'_, PyModule>) -> PyResult<()> {
    let py      = m.py();
    let members = PyDict::new(py);
    let mut idx = 0u32;
    while let Some(p) = ArbSamePriPolicy::from_index(idx) {
        members.set_item(p.variant_name(), idx)?;  // enum member: name = idx
        idx += 1;
    }
    let int_enum = py.import("enum")?.getattr("IntEnum")?;
    m.add("ArbSamePriPolicy", int_enum.call1(("ArbSamePriPolicy", &members))?)?;
    Ok(())
}

// Build `kathryn.FlowBlockType` as a Python IntEnum sourced entirely from the
// core `FlowBlockType` via `from_index` / `variant_name`. Bindings that cross
// the boundary (e.g. get_last_skeleton_flow_block_type) emit the same `to_index`,
// so the two sides share one source of truth and cannot misalign.
fn add_flow_block_type_enum(m: &Bound<'_, PyModule>) -> PyResult<()> {
    let py      = m.py();
    let members = PyDict::new(py);
    let mut idx = 0u32;
    while let Some(t) = FlowBlockType::from_index(idx) {
        members.set_item(t.variant_name(), idx)?;  // enum member: name = idx
        idx += 1;
    }
    let int_enum = py.import("enum")?.getattr("IntEnum")?;
    m.add("FlowBlockType", int_enum.call1(("FlowBlockType", &members))?)?;
    Ok(())
}

// Native extension module `_kathryn` — the pure-Python `kathryn` package
// (py/kathryn/) re-exports it. Registers every wrapper class + the LogicOp enum.
#[pymodule]
fn _kathryn(m: &Bound<'_, PyModule>) -> PyResult<()> {
    m.add_class::<PyModelArena>()?;
    m.add_class::<PyHcpIdent>()?;
    m.add_class::<PySlice>()?;
    m.add_class::<PyCcpIdent>()?;
    m.add_class::<PyFlowBlockIdent>()?;
    m.add_class::<PyModuleIdent>()?;
    m.add_class::<PyBackendVerilog>()?;
    add_logic_op_enum(m)?;
    add_arb_same_pri_policy_enum(m)?;
    add_flow_block_type_enum(m)?;
    add_asm_priority_consts(m)?;
    Ok(())
}
