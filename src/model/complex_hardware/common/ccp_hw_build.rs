use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::slice::Slice;
use crate::model::model_arena::ModelArena;

// Shared CCP wiring primitives — generic combinational helpers usable by any
// CCP's hardware build (Karray mux trees, DynCounter stage muxes, ...).

// Mux two sources into `dest_w` and commit the muxed event straight onto the
// destination's update pool (no asm node — AssignMeta::mux + final_update).
pub(crate) fn mux_into_wire(arena: &mut ModelArena, dest_w: HcpIdent, l_src: HcpIdent, r_src: HcpIdent, select_left: HcpIdent) {
    let width  = arena.get_hw_bit_sz(&dest_w);
    let mut lm = build_comb_asm_meta(arena, dest_w, l_src, width);
    let mut rm = build_comb_asm_meta(arena, dest_w, r_src, width);
    let muxed  = lm.mux(&mut rm, select_left, arena);
    muxed.final_update(arena);
}

// Build a combinational AssignMeta (ClkUnused, so it can feed AssignMeta::mux)
// driving `dest_i` from `src_i` over the full destination width.
fn build_comb_asm_meta(arena: &mut ModelArena, dest_i: HcpIdent, src_i: HcpIdent, width: i32) -> AssignMeta {
    let des      = arena.take_hcp(dest_i);
    let priority = des.get_priority();
    let uei      = des.gen_update_event_with_pri(src_i, None, Slice::new(0, width), priority, ClockMode::ClkUnused, arena);
    let am       = AssignMeta::new(dest_i, uei, ClockMode::ClkUnused);
    arena.replace_back_hcp(des);
    am
}
