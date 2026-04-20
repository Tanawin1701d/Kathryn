

use std::rc::Rc;
use crate::common::obj::SPTR;
use crate::model::hw_component::common::update_event::{UeBasic, UeCommon, UeType, UpdatingEvent};
use crate::model::hw_component::common::slice::Slice;
use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
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

    fn clone_hcp_asb_ident(&self) -> HcpIdent;

    // the global clock mode may not equal to the clock mode of this component
    fn retrieve_clk_mode(&self) -> ClockMode;
    ///fn get_ue_type(&self) -> UeType;
    fn get_des_slice(&self) -> Slice; /// typically it should start from 0
    fn get_priority(&self) -> i32;

    fn do_asm(&self,
              srci     : & HcpIdent,
              des_slice: & Option<Slice>,
              src_slice: & Slice,
              clk_mode : & Option<ClockMode>) -> AssignMeta;

    fn gen_update_event(&self,
                        srci     : & HcpIdent,
                        des_slice: & Option<Slice>,
                        src_slice: & Slice
    ) -> UeBasic {

        let std_des_slice = self.get_des_slice();
        let my_des_slice = des_slice.as_ref().unwrap_or(&std_des_slice);
        let my_src_slice = src_slice.clone();

        let resolved_des_slice = my_des_slice.get_match_size_sub_slice(&my_src_slice);
        let resolved_src_slice = my_src_slice.get_match_size_sub_slice(&my_des_slice);


        let mut ueb = UeBasic::new(srci.clone(), resolved_des_slice, resolved_src_slice);
        ueb.set_priority(self.get_priority());
        ueb.set_clk_mode(self.retrieve_clk_mode());
        return ueb;
    }

    fn gen_asm_meta(&self,
                    srci     : & HcpIdent,
                    des_slice: & Option<Slice>,
                    src_slice: & Slice,
    ) -> AssignMeta{
        
        let ueb = self.gen_update_event(
            srci,
            des_slice,
            src_slice
        );
        AssignMeta::new(self.clone_hcp_asb_ident(), ueb)
        
    }


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

impl HcpAssign {
    pub fn new() -> HcpAssign {
        HcpAssign {
            update_pool : UpdatePool::new()
        }
    }
}