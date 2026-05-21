use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::model_arena::ModelArena;
use crate::model::module::module_ident::ModuleIdent;

fn get_parent_module_ident(arena: &ModelArena, i: ModuleIdent) -> ModuleIdent {
    arena.get_module_ident_by_handle(i.get_master_module_handle())
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
        cur_a = get_parent_module_ident(arena, cur_a);
        cur_b = get_parent_module_ident(arena, cur_b);
        path_a.push(cur_a);
        path_b.push(cur_b);
    }


    (path_a, path_b)
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
