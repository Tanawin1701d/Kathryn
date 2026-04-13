use std::sync::atomic::{AtomicU64, Ordering};

static GLOBAL_MODEL_ID: AtomicU64 = AtomicU64::new(0);

pub fn get_last_ident_id() -> u64 {
    GLOBAL_MODEL_ID.load(Ordering::Relaxed)
}

pub struct IdentBase {
    global_id    : u64,
    global_name  : String,
    inherit_name : Vec<String>,
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

    fn get_inherit_name(&self) -> &Vec<String>           { &self.get_ident_base().inherit_name }
    fn set_inherit_name(&mut self, names: Vec<String>)   { self.get_ident_base_mut().inherit_name = names; }

    fn concat_inherit_name(&self) -> String {
        self.get_ident_base().concat_inherit_name()
    }
}

impl IdentBase {
    pub fn new() -> Self {
        Self {
            global_id    : GLOBAL_MODEL_ID.fetch_add(1, Ordering::Relaxed),
            global_name  : String::new(),
            inherit_name : Vec::new(),
        }
    }

    pub fn concat_inherit_name(&self) -> String {
        let pre_ret: String = self.inherit_name.iter()
            .map(|s| format!("{}_", s))
            .collect();
        assert!(pre_ret.len() > 1);
        pre_ret[..pre_ret.len() - 1].to_string() // remove the last "_"
    }

    /// Equivalent to C++ `operator=`: gets a fresh global ID and appends "_CP"
    /// to the name/inherit list.  Not `Clone` because the result is not identical.
    pub fn assign_from(&mut self, rhs: &IdentBase) {
        if std::ptr::eq(self, rhs) { return; }
        self.global_id    = GLOBAL_MODEL_ID.fetch_add(1, Ordering::Relaxed);
        self.global_name  = format!("{}_CP", rhs.global_name);
        self.inherit_name = rhs.inherit_name.clone();
        self.inherit_name.push("_CP".to_string());
    }
}
