use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::model_arena::ModelArena;

// taken_ident / taken_size: the HCP currently taken from the arena and its known
// bit width. If `ident` matches the taken slot the arena lookup is replaced with
// taken_size so the assertion still runs without touching the empty slot.
pub fn check_ident_bit_size(
    ident       : &HcpIdent,
    expected    : i32,
    owner_name  : &str,
    model_ar    : &ModelArena,
    taken_ident : Option<HcpIdent>,
    taken_size  : i32,
) {
    let sz = if taken_ident.map_or(false, |t| t == *ident) {
        taken_size
    } else {
        model_ar.get_hcp_assign(ident).get_des_slice().get_size()
    };
    assert_eq!(sz, expected,
               "'{}': signal '{}' must be {}-bit, got {}",
               owner_name, ident.get_ident_base().get_abs_name(), expected, sz);
}