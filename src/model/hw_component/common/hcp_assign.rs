

use std::rc::Rc;
use crate::common::obj::SPTR;
use crate::model::hw_component::common::update_event::UpdatingEvent;
use crate::model::hw_component::common::hcp_read::Readable;
use crate::model::hw_component::common::slice::Slice;
use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::update_pool::UpdatePool;

pub trait Assignable {

    /// ///////////////
    /// common function
    /// ///////////////

    /// ////////////////
    /// virtual function
    /// ////////////////

    fn get_hcp_assign(&self)         -> &    HcpAssign;
    fn get_hcp_assign_mut(&mut self) -> &mut HcpAssign;


    fn do_clk_edge_asm(&mut self, src_opr  : Rc<dyn Readable>,
                       des_slice: Slice,
                       clk_mode : ClockMode);

    fn do_level_asm(&mut self, src_opr  : Rc<dyn Readable>,
                    des_slice: Slice
    );

    fn add_update_event(&mut self, event: SPTR<dyn UpdatingEvent>){
        self.get_hcp_assign_mut().update_pool.add_update_event(event);
    }
    
    fn get_update_pool(&self) -> &UpdatePool {
        &self.get_hcp_assign().update_pool
    }
}

/*
   Hcp = Hw Component Property
*/

pub struct HcpAssign{
    update_pool : UpdatePool
}


