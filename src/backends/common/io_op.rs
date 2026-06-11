use crate::model::hw_component::common::hcp_ident::{HcpIdent, HwComponentType};
use crate::model::model_arena::{ModelArena, ModuleInitStage};
use crate::model::module::module_ident::ModuleIdent;


/// Creates an IoWire in `src_module` driven by `src_signal`.
/// `is_input` selects direction: `true` = input, `false` = output.
/// Pushes `src_module` onto the trace stack so the wire is stamped and
/// registered to the correct module, then pops on return.
pub fn build_io_wire(
    arena            : &mut ModelArena,
    target_module    : ModuleIdent,
    actual_src_signal: HcpIdent,
    agent_src_signal : HcpIdent,
    is_input         : bool,
) -> HcpIdent {
    let dir  = if is_input { "IN" } else { "OUT" };
    let name = format!("IO_{}_{}", dir, actual_src_signal.get_ident_base().get_abs_name());

    arena.push_module_trace_stack(target_module, ModuleInitStage::CompInit);
    let io_wire_i = arena.make_io_wire(false, &name, is_input, actual_src_signal, agent_src_signal);
    arena.pop_module_trace_stack();

    io_wire_i
}

/// Like `build_io_wire`, but the source signals are optional and `bit_width` is
/// given explicitly (the wire is left unbound when no agent source is present).
/// Constraint: an `actual_src_signal` may not be present without an
/// `agent_src_signal` — the actual source must always be reachable via an agent.
pub fn build_io_wire_opt_src(
    arena            : &mut ModelArena,
    target_module    : ModuleIdent,
    bit_width        : i32,
    actual_src_signal: Option<HcpIdent>,
    agent_src_signal : Option<HcpIdent>,
    is_input         : bool,
) -> HcpIdent {
    assert!(!(actual_src_signal.is_some() && agent_src_signal.is_none()),
            "build_io_wire_opt_src: actual_src_signal present but agent_src_signal missing");

    let dir  = if is_input { "IN" } else { "OUT" };
    let base = actual_src_signal
        .map        (|s| s.get_ident_base().get_abs_name().to_string())
        .unwrap_or_else(|| "anon".to_string());
    let name = format!("IO_{}_{}", dir, base);

    arena.push_module_trace_stack(target_module, ModuleInitStage::CompInit);
    let io_wire_i = arena.make_io_wire_opt_src(false, &name, is_input, bit_width, actual_src_signal, agent_src_signal);
    arena.pop_module_trace_stack();

    io_wire_i
}

/// Searches all IoWires registered on `module` (both user and internal) for one
/// that already binds `src_signal` in the requested direction.  Returns the
/// matching `HcpIdent` if found, `None` otherwise.
pub fn find_reusable_io_wire(
    arena      : &mut ModelArena,
    module     : ModuleIdent,
    actual_src_signal: HcpIdent,
    is_input   : bool,
) -> Option<HcpIdent> {
    // Snapshot IO wire ident lists so the module slot is free for HCP take/replace_back.
    let m = arena.take_module(module);
    let mut candidates = m.get_internal_hws(HwComponentType::IoWire).clone();
    candidates.extend_from_slice(m.get_user_hws(HwComponentType::IoWire));
    arena.replace_back_module(module, m);

    for io_wire_i in candidates {
        let io_wire = arena.take_io_wire(io_wire_i);
        let matched = io_wire.matches_signal(actual_src_signal, is_input);
        arena.replace_back_io_wire(io_wire);
        if matched {
            return Some(io_wire_i);
        }
    }
    None
}

