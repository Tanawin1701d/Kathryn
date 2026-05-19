use crate::model::common::identifier::Identifiable;
use crate::model::hw_component::common::hcp_ident::HcpIdent;

pub trait HcpIdentMutable: Identifiable {
    fn get_ident_mut(&mut self) -> &mut HcpIdent;
}
