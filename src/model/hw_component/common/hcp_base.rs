use crate::model::hw_component::common::hcp_assign::HcpAssignable;
use crate::model::hw_component::common::hcp_ident::HcpIdentifiable;
use crate::model::model_arena::ModelArena;

pub trait HcpBase: HcpAssignable + HcpIdentifiable {
    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena);
}