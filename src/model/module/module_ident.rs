use crate::model::common::identifier::{IdentBase, Identifiable};

// ---------------------------------------------------------------------------
// ModuleIdent — lightweight Copy handle for Module objects stored in
// ModelArena.  Mirrors the HcpIdent / UpdateEventIdent pattern.
// ---------------------------------------------------------------------------

#[derive(Clone, Copy, Debug, PartialEq, Eq, Default)]
pub struct ModuleIdent {
    ident_base : IdentBase,
}

impl ModuleIdent {
    pub fn new(is_user_com: bool, name: &str) -> Self {
        Self { ident_base: IdentBase::new(is_user_com, name) }
    }

    pub fn get_ident_base    (&self)     -> &IdentBase     { &self.ident_base }
    pub fn get_ident_base_mut(&mut self) -> &mut IdentBase { &mut self.ident_base }

    pub fn build_unique_module_name(&self) -> String {
        format!("MODULE_{}_{}",
                self.ident_base.get_name(),
                self.ident_base.get_global_id())
    }
}

impl Identifiable for ModuleIdent {
    fn get_ident_base    (&self)     -> &IdentBase     { &self.ident_base }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { &mut self.ident_base }
    fn build_unique_name (&mut self) -> &str {
        let name = self.build_unique_module_name();
        self.ident_base.set_name(&name);
        self.ident_base.get_name()
    }
}
