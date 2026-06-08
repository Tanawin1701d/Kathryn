// Python-facing module lifecycle ops. Mirrors the host module trace-stack
// helpers in `arena_impl.rs`. Minimal open/close surface: push a module onto
// the trace stack so subsequent component/flow-block construction attaches to
// it, then pop it and register it with its parent module.

use pyo3::prelude::*;
use super::model_arena::PyModelArena;
use super::module::module_ident_py::PyModuleIdent;
use crate::model::model_arena::ModuleInitStage;
use crate::model::module::module_ident::ModuleIdent;

#[pymethods]
impl PyModelArena {

    fn track_module_at_com_init(&mut self, module_i: PyModuleIdent) {
        self.arena.push_module_trace_stack(module_i.into(), ModuleInitStage::CompInit);
    }

    fn track_module_at_flow_init(&mut self, module_i: PyModuleIdent) {
        self.arena.push_module_trace_stack(module_i.into(), ModuleInitStage::FlowBlockInit);
    }

    // Close the active module: pop it (asserting it matches `module_i`) and
    // register it with the enclosing module via `add_user_sub_module`. The host
    // `stamp_module_to_parent_module` only sets the parent handle, so without
    // this the build DFS would never descend into the sub-module.
    fn untrack_module_at_com_init(&mut self, module_i: PyModuleIdent) {
        let module_i = self.pop_and_assert_module(module_i);
        if let Some((parent_i, _stage)) = self.arena.try_peek_module_trace_stack() {
            let mut parent = self.arena.take_module(parent_i);
            parent.add_user_sub_module(module_i);
            self.arena.replace_back_module(parent_i, parent);
        }
    }

    fn untrack_module_at_flow_init(&mut self, module_i: PyModuleIdent) {
        self.pop_and_assert_module(module_i);
    }


}

// ---- internal helpers (not exposed to Python) ----

impl PyModelArena {

    // Pop the active module off the trace stack, assert it matches the expected
    // ident, and return the resolved core ident for further use.
    fn pop_and_assert_module(&mut self, module_i: PyModuleIdent) -> ModuleIdent {
        let module_i: ModuleIdent = module_i.into();
        let (popped_i, _stage) = self.arena.pop_module_trace_stack();
        assert_eq!(popped_i, module_i, "finalize_module: ident mismatch — wrong module finalized");
        module_i
    }

}
