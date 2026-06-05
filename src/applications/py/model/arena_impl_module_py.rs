// Python-facing module lifecycle ops. Mirrors the host module trace-stack
// helpers in `arena_impl.rs`. Minimal open/close surface: push a module onto
// the trace stack so subsequent component/flow-block construction attaches to
// it, then pop it and register it with its parent module.

use pyo3::prelude::*;
use super::model_arena::PyModelArena;
use super::module::module_ident_py::PyModuleIdent;
use crate::model::model_arena::ModuleInitStage;

#[pymethods]
impl PyModelArena {
    // Open a module: push it onto the trace stack so subsequent HW components
    // and flow blocks attach to it. Pushed at `FlowBlockInit` — that stage lets
    // both HW declaration and top-flow-block finalization target this module
    // (the C++ component/flow two-phase split is unused while controller hooks
    // stay unported).
    fn initialize_module(&mut self, module_i: PyModuleIdent) {
        self.arena.push_module_trace_stack(module_i.into(), ModuleInitStage::FlowBlockInit);
    }

    // Close the active module: pop it (asserting it matches `module_i`) and
    // register it with the enclosing module via `add_user_sub_module`. The host
    // `stamp_module_to_parent_module` only sets the parent handle, so without
    // this the build DFS would never descend into the sub-module.
    fn finalize_module(&mut self, module_i: PyModuleIdent) {
        let module_i: crate::model::module::module_ident::ModuleIdent = module_i.into();
        let (popped_i, _stage) = self.arena.pop_module_trace_stack();
        assert_eq!(popped_i, module_i, "finalize_module: ident mismatch — wrong module finalized");

        if let Some((parent_i, _stage)) = self.arena.try_peek_module_trace_stack() {
            let mut parent = self.arena.take_module(parent_i);
            parent.add_user_sub_module(module_i);
            self.arena.replace_back_module(parent_i, parent);
        }
    }
}
