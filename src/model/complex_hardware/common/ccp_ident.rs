use crate::common::arena_base::ArenaHandle;
use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::module::module_ident::ModuleIdent;

// ---- CcpType ----------------------------------------------------------------

/// Kind of complex component property.  A CCP is a self-contained hardware
/// gadget (it owns its own wires/expressions) that, like an HCP, is stamped
/// into its owning module, but unlike an NCP is **not** part of the flow graph.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum CcpType {
    Arb,
    Karray,
    DynCounter,
}

impl CcpType {
    pub fn as_str(&self) -> &'static str {
        match self {
            CcpType::Arb        => "ARB",
            CcpType::Karray     => "KARRAY",
            CcpType::DynCounter => "DCNT",
        }
    }
}

impl std::fmt::Display for CcpType {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.write_str(self.as_str())
    }
}

// ---- CcpIdent ---------------------------------------------------------------

/// Lightweight Copy handle to a complex component property stored in the arena.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct CcpIdent {
    ident_base     : IdentBase,
    ccp_type       : CcpType,
    master_module_i: ModuleIdent,
}

impl CcpIdent {
    pub fn new(ccp_type: CcpType, is_user_com: bool, name: &str) -> Self {
        let mut s = Self {
            ident_base     : IdentBase::new(is_user_com),
            ccp_type,
            master_module_i: ModuleIdent::default(),
        };
        s.ident_base.set_rel_name(name);
        let abs = s.build_unique_ccp_name();
        s.ident_base.set_abs_name(&abs);
        s
    }

    pub fn get_ident_base     (&self)     -> &IdentBase     { &self.ident_base }
    pub fn get_ident_base_mut (&mut self) -> &mut IdentBase { &mut self.ident_base }
    pub fn get_ccp_type       (&self)     -> CcpType        { self.ccp_type }
    pub fn get_master_module_i(&self)     -> ModuleIdent    { self.master_module_i }
    pub fn set_master_module_i(&mut self, m: ModuleIdent)   { self.master_module_i = m; }

    pub fn build_unique_ccp_name(&self) -> String {
        format!("{}_{}_{}", self.ccp_type,
                self.ident_base.get_rel_name(),
                self.ident_base.get_global_id())
    }

    pub fn set_arena_handler(&mut self, arena_handler: ArenaHandle) {
        self.set_arena_handle(arena_handler);
    }
}

impl Identifiable for CcpIdent {
    fn get_ident_base    (&self)     -> &IdentBase     { &self.ident_base }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { &mut self.ident_base }
}
