// Global routing — routes signals that cross the whole module hierarchy up to
// the top module (counterpart to `internal_routing`, which threads signals only
// between a source and the lowest common ancestor of source and destination).
//
// Uses `graph::find_module_path_to_top` / `find_module_path_to_top_from_hcp`
// to obtain the full module path from an HCP's owning module up to the top.

use crate::backends::common::graph::find_module_path_to_top_from_hcp;
use crate::backends::common::io_op::{build_io_wire, build_io_wire_opt_src};
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::model_arena::ModelArena;
use crate::model::module::module_ident::ModuleIdent;

// Route `actual_src_i` from its own module up to the top module, building a
// fresh IoWire at each level along the path (no reuse lookup). `actual_src_i`
// must be IO-marked; the mark's direction selects how the chain is wired:
//
//   - Output: `actual_src_i` is the real source. The chain is built bottom-up
//     (own → top); each IoWire carries the actual source and is driven by the
//     wire one level below it (the lowest is driven by `actual_src_i` itself).
//
//   - Input: `actual_src_i` is the destination, not a source — so the chain has
//     no actual source signal. It is built top-down (top → own): the top port is
//     a primitive input (unbound) and each lower level is driven by the wire one
//     level above it.
//
// Returns `(nearest_io_i, top_io_i)`:
//   - nearest_io_i: the IoWire nearest the target HCP (built in its own module),
//   - top_io_i    : the IoWire on the top module.
pub fn route_glob_io_hw_comp(
    actual_src_i : HcpIdent,
    model_arena  : &mut ModelArena,
) -> (HcpIdent, HcpIdent) {
    // Direction comes from the IO mark — panic if the HCP is not IO-marked.
    let is_input = model_arena.get_io_mark(&actual_src_i)
        .expect("route_glob_io_hw_comp: expected an IO-marked component")
        .is_input();

    // Path from the HCP's own module up to the top (inclusive, own → top).
    let path = find_module_path_to_top_from_hcp(model_arena, actual_src_i);

    if is_input {
        route_glob_input(actual_src_i, &path, model_arena)
    } else {
        route_glob_output(actual_src_i, &path, model_arena)
    }
}

// Output side: bottom-up (own → top), actual source carried, agent = wire below.
fn route_glob_output(
    actual_src_i : HcpIdent,
    path         : &[ModuleIdent],
    model_arena  : &mut ModelArena,
) -> (HcpIdent, HcpIdent) {
    let mut agent_wire_i = actual_src_i;
    let mut nearest_io_i = None;
    for &module_i in path {
        agent_wire_i = build_io_wire(model_arena, module_i, actual_src_i, agent_wire_i, false);
        if nearest_io_i.is_none() {
            nearest_io_i = Some(agent_wire_i);   // first level = nearest to target
        }
    }

    let nearest_io_i = nearest_io_i.expect("route_glob_io_hw_comp: module path must be non-empty");
    let top_io_i     = agent_wire_i;             // last level = top module
    (nearest_io_i, top_io_i)
}

// Input side: top-down (top → own), no actual source, agent = wire above.
fn route_glob_input(
    actual_src_i : HcpIdent,
    path         : &[ModuleIdent],
    model_arena  : &mut ModelArena,
) -> (HcpIdent, HcpIdent) {
    let bit_width = model_arena.get_hw_bit_sz(&actual_src_i);

    let mut agent_wire_i : Option<HcpIdent> = None;   // top has no upper driver
    let mut top_io_i     = None;
    let mut nearest_io_i = actual_src_i;              // last built = own module
    for &module_i in path.iter().rev() {
        let io_i = build_io_wire_opt_src(model_arena, module_i, bit_width, None, agent_wire_i, true);
        if top_io_i.is_none() {
            top_io_i = Some(io_i);                    // first (top) level
        }
        agent_wire_i = Some(io_i);
        nearest_io_i = io_i;                          // overwritten until own module
    }

    let top_io_i = top_io_i.expect("route_glob_io_hw_comp: module path must be non-empty");
    (nearest_io_i, top_io_i)
}
