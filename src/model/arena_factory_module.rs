use crate::model::model_arena::{ModelArena, ModuleInitStage};
use crate::model::module::module::Module;
use crate::model::module::module_ident::ModuleIdent;

// make_* → is_user_com = false (internal/system)
// mk_*   → is_user_com = true  (user-defined)

impl ModelArena {
    fn stamp_module_to_parent_module(&mut self, mut i: ModuleIdent) -> ModuleIdent {
        if let Some((parent_i, _stage)) = self.try_peek_module_trace_stack() {
            i.set_master_module_i(parent_i);
            // Stack size == parent depth + 1, so it directly equals the child's depth.
            i.set_depth_level(self.module_trace_stack.len() as u32);
        }
        // Write the updated ident back into the Module stored in the arena.
        let mut m = self.take_module(i);
        m.set_ident(i);
        self.replace_back_module(i, m);
        i
    }

    pub fn mk_module(&mut self, name: &str) -> ModuleIdent {
        let i = self.add_module(Module::new(true, false, name));
        let i = self.stamp_module_to_parent_module(i);
        i
    }
    pub fn mk_top_module(&mut self, name: &str) -> ModuleIdent {
        let i = self.add_module(Module::new(true, true, name));
        self.push_module_trace_stack(i, ModuleInitStage::CompInit);
        i
    }
}
