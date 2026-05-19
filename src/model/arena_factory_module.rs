use crate::model::model_arena::ModelArena;
use crate::model::module::module::Module;
use crate::model::module::module_ident::ModuleIdent;

// make_* → is_user_com = false (internal/system)
// mk_*   → is_user_com = true  (user-defined)

impl ModelArena {
    fn stamp_module_to_parent_module(&mut self, mut i: ModuleIdent) -> ModuleIdent {
        if let Some((parent_i, _stage)) = self.try_peek_module_trace_stack() {
            i.set_master_module_i(parent_i);
        }
        i
    }

    pub fn mk_module(&mut self, name: &str) -> ModuleIdent {
        let i = self.add_module(Module::new(true, false, name));
        self.stamp_module_to_parent_module(i)
    }
    pub fn mk_top_module(&mut self, name: &str) -> ModuleIdent {
        self.add_module(Module::new(true, true, name))
    }
}
