use crate::model::model_arena::ModelArena;
use crate::model::module::module::Module;
use crate::model::module::module_ident::ModuleIdent;

// make_* → is_user_com = false (internal/system)
// mk_*   → is_user_com = true  (user-defined)

impl ModelArena {
    pub fn make_module(&mut self, name: &str) -> ModuleIdent {
        self.add_module(Module::new(false, false, name))
    }
    pub fn mk_module(&mut self, name: &str) -> ModuleIdent {
        self.add_module(Module::new(true, false, name))
    }
    pub fn mk_top_module(&mut self, name: &str) -> ModuleIdent {
        self.add_module(Module::new(true, true, name))
    }
}
