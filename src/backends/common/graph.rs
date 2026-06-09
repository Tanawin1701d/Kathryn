use crate::common::arena_base::ArenaHandle;
use crate::model::common::identifier::Identifiable;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::model_arena::ModelArena;
use crate::model::module::module_ident::ModuleIdent;

// ---- Module-tree traversal ----

/// Lazy DFS iterator over the module subtree rooted at `root_i`.
/// Yields each `ModuleIdent` in pre-order (parent before children,
/// siblings in declaration order).
///
/// The arena is **not** stored inside the struct — it is passed in on each
/// `next_module` call.  This means the arena borrow is held only for the
/// duration of one step, leaving it free for callers to use between steps.
///
/// ```ignore
/// let mut iter = DfsModuleIter::new(top_i);
/// while let Some(module_i) = iter.next_module(arena) {
///     do_work(module_i, arena);   // arena is free here
/// }
/// ```
pub struct DfsModuleIter {
    stack : Vec<ModuleIdent>,
}

impl DfsModuleIter {
    pub fn new(root_i: ModuleIdent) -> Self {
        Self { stack: vec![root_i] }
    }

    /// Advance one step; returns the next module or `None` when exhausted.
    pub fn next_module(&mut self, arena: &mut ModelArena) -> Option<ModuleIdent> {
        let cur_i  = self.stack.pop()?;
        let module = arena.take_module(cur_i);
        let children : Vec<ModuleIdent> = module.get_user_sub_modules().clone();
        arena.replace_back_module(cur_i, module);
        for &child_i in children.iter().rev() { self.stack.push(child_i); }
        Some(cur_i)
    }
}

fn get_parent_module_ident(arena: &ModelArena, i: ModuleIdent) -> ModuleIdent {
    let parent_handle = i.get_master_module_handle();
    // A top module carries the default handle; resolving it would deref the sentinel
    // ArenaHandle { 0, u32::MAX } and trip an opaque generation assert.  Fail loudly
    // here instead — this catches a stale depth_level whose parent handle is unset.
    assert_ne!(
        parent_handle, ArenaHandle::default(),
        "get_parent_module_ident: module \"{}\" (id {}) is a top module and has no \
         parent — module tree / depth_level is inconsistent",
        i.get_global_name(), i.get_global_id(),
    );
    arena.get_module_ident_by_handle(parent_handle)
}

/// Returns `(path_a, path_b)` where each path walks from the given module up
/// to the lowest common ancestor (inclusive).  `depth_level` is used to
/// balance the two sides before the joint walk.
pub fn find_common_ancestor_module_paths(
    arena  : &ModelArena,
    a      : ModuleIdent,
    b      : ModuleIdent,
) -> (Vec<ModuleIdent>, Vec<ModuleIdent>) {
    let mut cur_a  = a;
    let mut cur_b  = b;
    let mut path_a = vec![cur_a];
    let mut path_b = vec![cur_b];
    // If a == b the three loops below are all no-ops and we return immediately.

    // Balance: bring the deeper side up to match the shallower depth.
    while cur_a.get_depth_level() > cur_b.get_depth_level() {
        cur_a = get_parent_module_ident(arena, cur_a);
        path_a.push(cur_a);
    }
    while cur_b.get_depth_level() > cur_a.get_depth_level() {
        cur_b = get_parent_module_ident(arena, cur_b);
        path_b.push(cur_b);
    }

    // Walk both up together until they meet at the common ancestor.
    while cur_a != cur_b {
        // The balance loops left both sides at equal depth, and each step keeps them
        // level.  Reaching a top module (depth 0) while still unequal means the two
        // modules live in different trees — no common ancestor exists, and stepping
        // further would deref the default parent handle.
        assert!(
            cur_a.get_depth_level() > 0,
            "find_common_ancestor_module_paths: modules \"{}\" (id {}) and \"{}\" (id {}) \
             share no common ancestor — they are in different module trees",
            a.get_global_name(), a.get_global_id(),
            b.get_global_name(), b.get_global_id(),
        );
        cur_a = get_parent_module_ident(arena, cur_a);
        cur_b = get_parent_module_ident(arena, cur_b);
        path_a.push(cur_a);
        path_b.push(cur_b);
    }


    (path_a, path_b)
}

/// Walk from `module_i` up to the top module (inclusive), returning the path
/// ordered from the input module up to the top. The top module has
/// `depth_level == 0` (no parent), so it is the last element.
pub fn find_module_path_to_top(
    arena    : &ModelArena,
    module_i : ModuleIdent,
) -> Vec<ModuleIdent> {
    let mut cur_i = module_i;
    let mut path  = vec![cur_i];
    while cur_i.get_depth_level() > 0 {
        cur_i = get_parent_module_ident(arena, cur_i);
        path.push(cur_i);
    }
    path
}

/// Convenience wrapper: resolves the master module of each HcpIdent, then
/// delegates to `find_common_ancestor_module_paths`.
pub fn find_common_ancestor_module_paths_from_hcp(
    arena : &ModelArena,
    a     : HcpIdent,
    b     : HcpIdent,
) -> (Vec<ModuleIdent>, Vec<ModuleIdent>) {
    find_common_ancestor_module_paths(arena, a.get_master_module_i(), b.get_master_module_i())
}

/// Convenience wrapper: resolves the master module of `hcp_i`, then delegates
/// to `find_module_path_to_top`.
pub fn find_module_path_to_top_from_hcp(
    arena : &ModelArena,
    hcp_i : HcpIdent,
) -> Vec<ModuleIdent> {
    find_module_path_to_top(arena, hcp_i.get_master_module_i())
}