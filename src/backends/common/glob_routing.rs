// Global routing — routes signals that cross the whole module hierarchy up to
// the top module (counterpart to `internal_routing`, which threads signals only
// between a source and the lowest common ancestor of source and destination).
//
// Uses `graph::find_module_path_to_top` / `find_module_path_to_top_from_hcp`
// to obtain the full module path from an HCP's owning module up to the top.

use crate::backends::common::graph::{find_module_path_to_top_from_hcp, DfsModuleIter};
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
    actual_target_i: HcpIdent,
    model_arena  : &mut ModelArena,
) -> (HcpIdent, HcpIdent) {
    // Direction comes from the IO mark — panic if the HCP is not IO-marked.
    let is_input = model_arena.get_io_mark(&actual_target_i)
        .expect("route_glob_io_hw_comp: expected an IO-marked component")
        .is_input();

    // Path from the HCP's own module up to the top (inclusive, own → top).
    let path = find_module_path_to_top_from_hcp(model_arena, actual_target_i);

    let (nearest_io_i, top_io_i) = if is_input {
        route_glob_input(actual_target_i, &path, model_arena)
    } else {
        route_glob_output(actual_target_i, &path, model_arena)
    };

    // Name the top port after the target's IO mark so the emitted Verilog port
    // carries the user-facing name rather than an auto name.
    let io_name = model_arena.get_io_mark(&actual_target_i)
        .expect("route_glob_io_hw_comp: target must be IO-marked")
        .io_name()
        .to_string();
    let mut top_io = model_arena.take_io_wire(top_io_i);
    top_io.set_explicit_name(io_name);
    model_arena.replace_back_io_wire(top_io);

    (nearest_io_i, top_io_i)
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
    let mut top_io_i     : Option<HcpIdent> = None;
    let mut nearest_io_i : Option<HcpIdent> = None;   // last built = own module
    for &module_i in path.iter().rev() {
        let io_i = build_io_wire_opt_src(model_arena, module_i, bit_width, None, agent_wire_i, true);
        if top_io_i.is_none() {
            top_io_i = Some(io_i);                    // first (top) level
        }
        agent_wire_i = Some(io_i);
        nearest_io_i = Some(io_i);                          // overwritten until own module
    }

    let nearest_io_i = nearest_io_i.expect("route_glob_io_hw_comp: module path must be non-empty");
    let top_io_i     = top_io_i    .expect("route_glob_io_hw_comp: module path must be non-empty");

    // Drive the target from the nearest IoWire: the target is the chain's
    // destination, so generate an update event into its update pool.
    let src_slice  = model_arena.get_hw_slice(&nearest_io_i);
    let mut target = model_arena.take_hcp(actual_src_i);
    target.bind_src(nearest_io_i, None, src_slice, model_arena);
    model_arena.replace_back_hcp(target);

    (nearest_io_i, top_io_i)
}


// Top-level entry point: DFS the full module tree and route every IO-marked HCP
// up to the top module. Requires a top module to be set on the arena.
pub(crate) fn route_glob_io_model(model_arena: &mut ModelArena) {
    let top_i = model_arena.get_top_module()
        .expect("route_glob_io_model: no top module set");

    let mut iter = DfsModuleIter::new(top_i);
    while let Some(module_i) = iter.next_module(model_arena) {
        // Collect this module's IO-marked HCPs, then route each to the top.
        let module = model_arena.take_module(module_i);
        let mut io_marked_i: Vec<HcpIdent> = Vec::new();
        module.gather_io_marked_hcps(model_arena, &mut io_marked_i);
        model_arena.replace_back_module(module_i, module);

        for target_i in io_marked_i {
            route_glob_io_hw_comp(target_i, model_arena);
        }
    }
}