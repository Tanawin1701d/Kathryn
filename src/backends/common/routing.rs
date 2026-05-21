use crate::backends::common::io_op::{build_io_wire, find_reusable_io_wire};
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::model_arena::ModelArena;
use crate::model::module::module_ident::ModuleIdent;

// Creates IoWires along both sides of a cross-module path.
// output_paths and input_paths come from find_common_ancestor_module_paths;
// the last element of each is the LCA (shared crossing point) and is skipped.
fn route_io_base(
    input_paths      : &mut Vec<ModuleIdent>,
    output_paths     : &mut Vec<ModuleIdent>,
    des_i            : HcpIdent,
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
    // Each level's agent is the IoWire produced one level below it.
    let mut output_agent_wire_i = actual_src_i;
    let output_len = output_paths.len().saturating_sub(1);
    for &module_i in &output_paths[..output_len] {
        output_agent_wire_i = find_reusable_io_wire(model_arena, module_i, actual_src_i, false)
            .unwrap_or_else(
                || build_io_wire(model_arena, module_i, actual_src_i, output_agent_wire_i, false)
            );
    }

    // ---- Input side (LCA → destination): import actual_src_i downward ----
    // Iterate from just-below-LCA (back-1) toward dest (front); seed agent from the output chain's top wire.
    let mut input_agent_wire_i = output_agent_wire_i;
    let input_back1_to_front = (0..input_paths.len().saturating_sub(1)).rev();
    for idx in input_back1_to_front {
        let module_i = input_paths[idx];
        input_agent_wire_i = find_reusable_io_wire(model_arena, module_i, actual_src_i, true)
            .unwrap_or_else(|| build_io_wire(model_arena, module_i, actual_src_i, input_agent_wire_i, true));
    }

    input_agent_wire_i
}


fn route_io_module(
    module_i   : ModuleIdent,
    model_arena:  &mut ModelArena

){

}