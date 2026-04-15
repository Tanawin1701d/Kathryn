use std::sync::atomic::{AtomicU64, Ordering};

static GLOBAL_MODEL_ID: AtomicU64 = AtomicU64::new(0);

pub fn get_last_ident_id() -> u64 {
    GLOBAL_MODEL_ID.load(Ordering::Relaxed)
}

pub struct IdentBase {
    global_id    : u64,
    global_name  : String,
}

/// Trait for types that embed `IdentBase` and implement the pure-virtual
/// `build_inherit_name`.  Mirrors the C++ `IdentBase` abstract class.
pub trait HasIdentBase {
    fn get_ident_base    (&self)     -> &IdentBase;
    fn get_ident_base_mut(&mut self) -> &mut IdentBase;
    fn build_inherit_name(&mut self);

    // ---- forwarded accessors ------------------------------------------------


    fn get_global_id  (&self) -> u64    { self.get_ident_base().global_id }

    fn get_global_name(&self) -> &str          { &self.get_ident_base().global_name }
    fn set_global_name(&mut self, name: String) { self.get_ident_base_mut().global_name = name; }
}

impl IdentBase {
    pub fn new() -> Self {
        Self {
            global_id    : GLOBAL_MODEL_ID.fetch_add(1, Ordering::Relaxed),
            global_name  : String::new(),
        }
    }

    /// Equivalent to C++ `operator=`: gets a fresh global ID and appends "_CP"
    /// to the name/inherit list.  Not `Clone` because the result is not identical.
    pub fn assign_from(&mut self, rhs: &IdentBase) {
        if std::ptr::eq(self, rhs) { return; }
        self.global_id    = GLOBAL_MODEL_ID.fetch_add(1, Ordering::Relaxed);
        self.global_name  = format!("{}_CP", rhs.global_name);
    }
}
