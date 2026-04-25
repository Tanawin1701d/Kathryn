use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::model_arena::ModelArena;

pub fn check_ident_bit_size(ident: &HcpIdent, expected: i32, owner_name: &str, model_ar: &ModelArena) {
    let sz = model_ar.get_hcp_assign(ident).get_des_slice().get_size();
    assert_eq!(sz, expected,
        "'{}': signal '{}' must be {}-bit, got {}",
        owner_name, ident.get_ident_base().get_name(), expected, sz);
}