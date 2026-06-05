// PyO3 binding root. Mirrors the host module tree under `py/` and exposes the
// Python module entry point. Every Python macro in the crate lives under here.

use pyo3::prelude::*;
use pyo3::types::PyDict;

pub mod model;

use model::model_arena::PyModelArena;
use model::hw_component::common::hcp_ident_py::PyHcpIdent;
use model::hw_component::common::slice_py::PySlice;
use model::flow_block::flow_block_ident_py::PyFlowBlockIdent;
use model::module::module_ident_py::PyModuleIdent;
use crate::model::hw_component::common::operation::LogicOp;

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

// `import kathryn` — registers the wrappers exposed so far (phase 1).
#[pymodule]
fn kathryn(m: &Bound<'_, PyModule>) -> PyResult<()> {
    m.add_class::<PyModelArena>()?;
    m.add_class::<PyHcpIdent>()?;
    m.add_class::<PySlice>()?;
    m.add_class::<PyFlowBlockIdent>()?;
    m.add_class::<PyModuleIdent>()?;
    add_logic_op_enum(m)?;
    Ok(())
}
