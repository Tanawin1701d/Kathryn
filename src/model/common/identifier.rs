use std::sync::atomic::{AtomicU64, Ordering};
use crate::common::arena_base::ArenaHandle;

static GLOBAL_MODEL_ID: AtomicU64 = AtomicU64::new(0);

pub fn get_last_ident_id() -> u64 {
    GLOBAL_MODEL_ID.load(Ordering::Relaxed)
}

#[derive(Clone, Debug, Eq)]
pub struct IdentBase {
    global_id    : u64,
    name         : String,
    arena_handle : ArenaHandle,

}

/// Trait for types that embed `IdentBase` and implement the pure-virtual
/// `build_inherit_name`.  Mirrors the C++ `IdentBase` abstract class.
pub trait Identifiable {
    fn get_ident_base    (&self)     -> &IdentBase;
    fn get_ident_base_mut(&mut self) -> &mut IdentBase;
    fn build_unique_name(&mut self)  -> &str;

    // ---- forwarded accessors ------------------------------------------------


    fn get_global_id  (&self) -> u64    { self.get_ident_base().global_id }

    fn get_global_name(&self) -> &str          { &self.get_ident_base().name }
    fn set_global_name(&mut self, name: String) { self.get_ident_base_mut().name = name; }

    fn get_arena_handle(&self) -> &ArenaHandle { &self.get_ident_base().arena_handle }
    fn set_arena_handle(&mut self, arena_handle: ArenaHandle) { self.get_ident_base_mut().arena_handle = arena_handle; }
}

impl IdentBase {
    pub fn new(name: &str) -> Self {
        Self {
            global_id : GLOBAL_MODEL_ID.fetch_add(1, Ordering::Relaxed),
            name: name.to_string(),
            arena_handle: ArenaHandle::default(),
        }
    }

    /// Equivalent to C++ `operator=`: gets a fresh global ID and appends "_CP"
    /// to the name/inherit list.  Not `Clone` because the result is not identical.
    pub fn assign_from(&mut self, rhs: &IdentBase) {
        if std::ptr::eq(self, rhs) { return; }
        self.global_id = GLOBAL_MODEL_ID.fetch_add(1, Ordering::Relaxed);
        self.name      = format!("{}_CP", rhs.name);
    }

    pub fn get_global_id(&self)      -> u64         { self.global_id }
    pub fn get_name     (&self)      -> &str        { &self.name }
    pub fn get_name_mut (&mut self)  -> &mut String { &mut self.name }
    pub fn set_name     (&mut self, name: &str)     { self.name = name.to_string(); }
}

impl PartialEq for IdentBase {
    fn eq(&self, other: &Self) -> bool {
        self.global_id == other.global_id && self.name == other.name
    }
}
