

use std::rc::Rc;
use crate::common::obj::SPTR;
use crate::model::hw_component::common::update_event::{UeBasic, UpdatingEvent};
use crate::model::hw_component::common::hcp_read::Readable;
use crate::model::hw_component::common::slice::Slice;
use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::update_pool::UpdatePool;

pub trait HcpAssignable {

    /// ///////////////
    /// common function
    /// ///////////////

    /// ////////////////
    /// virtual function
    /// ////////////////

    fn get_hcp_assign(&self)         -> &    HcpAssign;
    fn get_hcp_assign_mut(&mut self) -> &mut HcpAssign;

    // the global clock mode may not equal to the clock mode of this component
    fn retrieve_clk_mode(&self) -> ClockMode;

    fn do_asm(self: SPTR<Self>,
              src_opr  : SPTR<dyn Readable>,
              des_slice: Option<Slice>,
              clk_mode : Option<ClockMode>);

    fn gen_update_event(&self,
                        src_opr  : SPTR<dyn Readable>,
                        des_slice: Slice,
                        clk_mode : ClockMode,

    ) -> UeBasic;

    fn gen_asm_meta(&self,
                    src_opr  : SPTR<dyn Readable>,
                    des_slice: Option<Slice>,
                    clk_mode : Option<ClockMode>
    ) -> AssignMeta;


    fn add_update_event(&mut self, event: Box<dyn UpdatingEvent>){
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


