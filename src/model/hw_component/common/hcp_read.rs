use crate::model::hw_component::common::hcp_ident::HcpIdent;

pub trait HcpReadable {

    fn clone_hcp_rdb_ident(&self) -> HcpIdent;

}





