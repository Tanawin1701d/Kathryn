use crate::common::arena_base::ArenaHandle;
use crate::model::common::identifier::{IdentBase, Identifiable};

// ---------------------------------------------------------------------------
// ModuleIdent — lightweight Copy handle for Module objects stored in
// ModelArena.  Mirrors the HcpIdent / UpdateEventIdent pattern.
// ---------------------------------------------------------------------------

#[derive(Clone, Copy, Debug, PartialEq, Eq, Default)]
pub struct ModuleIdent {
    ident_base           : IdentBase,
    // ArenaHandle of the parent module; default() means "no parent" (top module).
    master_module_handle : ArenaHandle,
    // Nesting depth: 0 = top module, +1 per level of sub-module.
    depth_level          : u32,
}

impl ModuleIdent {
    pub fn new(is_user_com: bool, name: &str) -> Self {
        Self { ident_base: IdentBase::new(is_user_com, name), master_module_handle: ArenaHandle::default(), depth_level: 0 }
    }

    pub fn get_ident_base          (&    self                     ) -> &IdentBase     { &self.ident_base                                        }
    pub fn get_ident_base_mut      (&mut self                     ) -> &mut IdentBase { &mut self.ident_base                                    }
    pub fn get_master_module_handle(&    self                     ) -> ArenaHandle    { self.master_module_handle                               }
    pub fn set_master_module_i     (&mut self, parent: ModuleIdent)                   { self.master_module_handle = *parent.get_arena_handle(); }
    pub fn get_depth_level         (&    self                     ) -> u32            { self.depth_level                                        }
    pub fn set_depth_level         (&mut self, level: u32         )                   { self.depth_level = level;                               }

    pub fn build_unique_module_name(&self) -> String {
        format!("MODULE_{}_{}",
                self.ident_base.get_abs_name(),
                self.ident_base.get_global_id())
    }
}

impl Identifiable for ModuleIdent {
    fn get_ident_base    (&self)     -> &IdentBase     { &self.ident_base }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { &mut self.ident_base }
    fn build_unique_name (&mut self) -> &str {
        let name = self.build_unique_module_name();
        self.ident_base.set_abs_name(&name);
        self.ident_base.get_abs_name()
    }
}
