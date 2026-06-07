// Global routing — routes signals that cross the whole module hierarchy up to
// the top module (counterpart to `internal_routing`, which threads signals only
// between a source and the lowest common ancestor of source and destination).
//
// Uses `graph::find_module_path_to_top` / `find_module_path_to_top_from_hcp`
// to obtain the full module path from an HCP's owning module up to the top.
