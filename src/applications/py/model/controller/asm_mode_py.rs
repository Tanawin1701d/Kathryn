// Python-facing asm-node priority controls. Mirrors `src/model/controller/asm_mode.rs`.
// The priority state is a thread-local global (not arena-bound), but we expose it
// as PyModelArena methods so the DSL reaches it through the same single `arena()`
// entry point as everything else. The DEFAULT_UE_PRI_* constants are registered as
// module-level attributes by `add_asm_priority_consts`.

use pyo3::prelude::*;
use super::super::model_arena::PyModelArena;
use crate::model::controller::asm_mode::{
    AsmNodePriorityMode,
    get_asm_pri_mode, get_asm_pri_val,
    set_asm_pri_to_auto, set_asm_pri_to_manual,
};
use crate::model::hw_component::common::update_event::asm_priority_consts;

#[pymethods]
impl PyModelArena {
    // Current priority mode as a string ("Auto" / "Manual"). String mirrors the
    // host enum without crossing a second enum over the boundary.
    fn get_asm_pri_mode(&self) -> &'static str {
        match get_asm_pri_mode() {
            AsmNodePriorityMode::Auto   => "Auto",
            AsmNodePriorityMode::Manual => "Manual",
        }
    }

    // Current priority value applied to subsequently-built update events.
    fn get_asm_pri_val(&self) -> i32 {
        get_asm_pri_val()
    }

    // Switch back to auto mode (priority resets to DEFAULT_UE_PRI_USER).
    fn set_asm_pri_to_auto(&mut self) {
        set_asm_pri_to_auto();
    }

    // Pin every subsequently-built update event to `priority` until changed.
    fn set_asm_pri_to_manual(&mut self, priority: i32) {
        set_asm_pri_to_manual(priority);
    }
}

// Register every UE-priority constant on `_kathryn` by walking the host's
// `asm_priority_consts()` table — the single source of truth. Adding a row to the
// host macro auto-registers it here; nothing is duplicated. Also publishes the
// authoritative name list as `_ASM_PRIORITY_CONST_NAMES` so the Python layer can
// re-export the set without hardcoding any names either.
pub fn add_asm_priority_consts(m: &Bound<'_, PyModule>) -> PyResult<()> {
    let mut names: Vec<&str> = Vec::new();
    for (name, val) in asm_priority_consts() {
        m.add(*name, *val)?;
        names.push(name);
    }
    m.add("_ASM_PRIORITY_CONST_NAMES", names)?;
    Ok(())
}
