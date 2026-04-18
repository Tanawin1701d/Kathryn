use crate::model::hw_component::common::hcp_assign::HcpAssignable;
use crate::model::hw_component::common::hcp_ident::HcpIdent;

pub struct ModelArena{


}

impl ModelArena{

    pub fn new() -> Self{
        Self{}
    }

    pub fn borrow_asb_mut(&mut self, hcp_ident: HcpIdent) -> &mut dyn HcpAssignable{

    }

}