use std::collections::{HashMap, HashSet};
use crate::backends::common::graph::{find_common_ancestor_module_paths, DfsModuleIter};
use crate::backends::common::io_op::{build_io_wire, find_reusable_io_wire};
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::model_arena::ModelArena;
use crate::model::module::module_ident::ModuleIdent;

// Creates IoWires along both sides of a cross-module path. output_paths and
// input_paths come from find_common_ancestor_module_paths.
//
// IoWire chain built (every hop binds the SAME actual_src_i; only agent_src —
// the immediate driver — changes; ▲ / ◄ = "agent drives this wire"):
//
//                           [ LCA ]      last element of BOTH paths — SKIPPED
//                          ╱       ╲     (no wire built in it; the two chains
//          out-leg (rises)          ╲     meet directly across it)
//                        ╱   seed    ╲
//         ┌───────────┐    edge       ┌───────────┐
//         │  mid_out  │ io_out2 ────► │  mid_in   │ io_in1
//         └───────────┘               └───────────┘
//               ▲ agent                     │ agent
//         ┌───────────┐               ┌───────────┐
//         │  src_mod  │ io_out1       │  des_mod  │ io_in2  ◄── returned ident
//         └───────────┘               └───────────┘
//               ▲ agent
//          actual_src_i  (the HCP being exported)
//
// - out-leg: output_paths walked front→back (source upward); each level's agent
//   is the io_out built ONE LEVEL BELOW (seeded by actual_src_i itself).
// - in-leg : input_paths walked back-1→front (just-below-LCA downward); each
//   level's agent is the io_in ONE LEVEL ABOVE (seeded by the out-leg's TOP wire).
// - reuse  : every hop consults find_reusable_io_wire(actual_src_i, direction)
//   first, so one exported signal never grows two parallel chains.
fn route_io_base(
    input_paths      : &mut Vec<ModuleIdent>,
    output_paths     : &mut Vec<ModuleIdent>,
    actual_src_i     : HcpIdent,
    model_arena      :  &mut ModelArena,
) -> HcpIdent {
    assert!(!input_paths.is_empty() && !output_paths.is_empty(),
            "route_io_base: path vectors must not be empty");

    // Both single-element means src and des are already in the same module (the LCA).
    if input_paths.len() == 1 && output_paths.len() == 1 {
        assert_eq!(input_paths[0], output_paths[0],
                   "route_io_base: single-element paths must refer to the same module (LCA)");
        return actual_src_i;
    }

    // ---- Output side (source → LCA): export actual_src_i upward ----
    let mut output_agent_wire_i = actual_src_i;
    let output_len = output_paths.len().saturating_sub(1);
    for &module_i in &output_paths[..output_len] {
        output_agent_wire_i = find_reusable_io_wire(model_arena, module_i, actual_src_i, false)
            .unwrap_or_else(
                || build_io_wire(model_arena, module_i, actual_src_i, output_agent_wire_i, false)
            );
    }

    // ---- Input side (LCA → destination): import actual_src_i downward ----
    let mut input_agent_wire_i = output_agent_wire_i;
    let input_back1_to_front = (0..input_paths.len().saturating_sub(1)).rev();
    for idx in input_back1_to_front {
        let module_i = input_paths[idx];
        input_agent_wire_i = find_reusable_io_wire(model_arena, module_i, actual_src_i, true)
            .unwrap_or_else(|| build_io_wire(model_arena, module_i, actual_src_i, input_agent_wire_i, true));
    }

    input_agent_wire_i
}

fn route_io_hw_comp(
    actual_src_i : HcpIdent,
    des_mod_i    : ModuleIdent,
    model_arena  : &mut ModelArena,
) -> HcpIdent {
    let src_mod_i = actual_src_i.get_master_module_i();
    let (mut input_paths, mut output_paths) =
        find_common_ancestor_module_paths(model_arena, des_mod_i, src_mod_i);
    // new input io for the des module
    let new_des_io_i = route_io_base(&mut input_paths, &mut output_paths, actual_src_i, model_arena);
    new_des_io_i
}


fn route_and_remap_io_module(
    module_i    : ModuleIdent,
    model_arena : &mut ModelArena,
) {
    // 1. gather all deps from every HCP in this module (module taken only for the read)
    let module = model_arena.take_module(module_i);
    let mut deps: HashSet<HcpIdent> = HashSet::new();
    module.gather_dep_hcps(model_arena, &mut deps);
    model_arena.replace_back_module(module_i, module);

    // 2. route each cross-module dep; build old → new-io-wire map.
    //    module_i must NOT be held taken here — the routing path walks back
    //    through module_i itself (find_reusable_io_wire re-takes it).
    let mut remap: HashMap<HcpIdent, HcpIdent> = HashMap::new();
    for dep_i in deps {
        if dep_i.get_master_module_i() != module_i {
            let io_wire_i = route_io_hw_comp(dep_i, module_i, model_arena);
            remap.insert(dep_i, io_wire_i);
        }
    }

    // 3. rewrite all dep handles inside the module to point at the new IoWires
    let module = model_arena.take_module(module_i);
    module.remap_dep_hcps(&remap, model_arena);
    model_arena.replace_back_module(module_i, module);
}


pub(crate) fn route_and_remap_io_model(model_arena: &mut ModelArena) {
    let top_i = model_arena.get_top_module()
        .expect("route_and_remap_io_model: no top module set");

    let mut iter = DfsModuleIter::new(top_i);
    while let Some(module_i) = iter.next_module(model_arena) {
        route_and_remap_io_module(module_i, model_arena);
    }
}