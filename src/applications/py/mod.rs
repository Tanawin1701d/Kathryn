// PyO3 binding root. Mirrors the host module tree under `py/` and exposes the
// Python module entry point. Every Python macro in the crate lives under here.

use pyo3::prelude::*;

pub mod model;

use model::model_arena::PyModelArena;
use model::hw_component::common::hcp_ident::PyHcpIdent;
use model::hw_component::common::slice::PySlice;

// `import kathryn` — registers the wrappers exposed so far (phase 1).
#[pymodule]
fn kathryn(m: &Bound<'_, PyModule>) -> PyResult<()> {
    m.add_class::<PyModelArena>()?;
    m.add_class::<PyHcpIdent>()?;
    m.add_class::<PySlice>()?;
    Ok(())
}
