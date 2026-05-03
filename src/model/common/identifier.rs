use std::sync::atomic::{AtomicU64, Ordering};
use crate::common::arena_base::ArenaHandle;

static GLOBAL_MODEL_ID: AtomicU64 = AtomicU64::new(0);

pub const MAX_NAME_LEN: usize = 128;

pub fn get_last_ident_id() -> u64 {
    GLOBAL_MODEL_ID.load(Ordering::Relaxed)
}

#[derive(Clone, Copy, Debug, Eq)]
pub struct IdentBase {
    global_id   : u64,
    is_user_com : bool,
    name_buf    : [u8; MAX_NAME_LEN],
    name_len    : u8,
    arena_handle: ArenaHandle,
}

impl Default for IdentBase {
    fn default() -> Self {
        Self {
            global_id   : 0,
            is_user_com : false,
            name_buf    : [0u8; MAX_NAME_LEN],
            name_len    : 0,
            arena_handle: ArenaHandle::default(),
        }
    }
}

/// Trait for types that embed `IdentBase` and implement the pure-virtual
/// `build_inherit_name`.  Mirrors the C++ `IdentBase` abstract class.
pub trait Identifiable {
    fn get_ident_base    (&self)     -> &IdentBase;
    fn get_ident_base_mut(&mut self) -> &mut IdentBase;
    fn build_unique_name(&mut self)  -> &str;

    fn get_global_id  (&self) -> u64  { self.get_ident_base().global_id }
    fn get_global_name(&self) -> &str { self.get_ident_base().get_name() }
    fn set_global_name(&mut self, name: &str) { self.get_ident_base_mut().set_name(name); }
    fn get_is_user_com(&self) -> bool { self.get_ident_base().is_user_com }
    fn get_arena_handle(&self) -> &ArenaHandle { &self.get_ident_base().arena_handle }
    fn set_arena_handle(&mut self, arena_handle: ArenaHandle) {
        self.get_ident_base_mut().arena_handle = arena_handle;
    }
}

impl IdentBase {
    pub fn new(is_user_com: bool, name: &str) -> Self {
        let mut base = Self {
            global_id   : GLOBAL_MODEL_ID.fetch_add(1, Ordering::Relaxed),
            is_user_com,
            name_buf    : [0u8; MAX_NAME_LEN],
            name_len    : 0,
            arena_handle: ArenaHandle::default(),
        };
        base.set_name(name);
        base
    }

    /// Gets a fresh global ID and appends "_CP" to the name.
    /// Not `Clone` because the result is not identical.
    pub fn assign_from(&mut self, rhs: &IdentBase) {
        if std::ptr::eq(self, rhs) { return; }
        self.global_id = GLOBAL_MODEL_ID.fetch_add(1, Ordering::Relaxed);
        let cp = format!("{}_CP", rhs.get_name());
        self.set_name(&cp);
    }

    pub fn get_global_id  (&self) -> u64  { self.global_id }
    pub fn get_is_user_com(&self) -> bool { self.is_user_com }

    pub fn get_name(&self) -> &str {
        std::str::from_utf8(&self.name_buf[..self.name_len as usize]).unwrap()
    }

    pub fn set_name(&mut self, name: &str) {
        let bytes = name.as_bytes();
        assert!(bytes.len() <= MAX_NAME_LEN, "name exceeds MAX_NAME_LEN ({}): \"{}\"", MAX_NAME_LEN, name);
        self.name_buf[..bytes.len()].copy_from_slice(bytes);
        self.name_len = bytes.len() as u8;
    }
}

impl PartialEq for IdentBase {
    fn eq(&self, other: &Self) -> bool {
        self.global_id == other.global_id && self.get_name() == other.get_name()
    }
}
