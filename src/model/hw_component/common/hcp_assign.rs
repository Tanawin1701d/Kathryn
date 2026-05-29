

use crate::model::hw_component::common::slice::Slice;
use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::update_event_ident::UpdateEventIdent;
use crate::model::hw_component::common::update_pool::UpdatePool;
use crate::model::model_arena::ModelArena;

pub trait HcpAssignable {

    /// ///////////////
    /// common function
    /// ///////////////

    /// ////////////////
    /// virtual function
    /// ////////////////

    fn get_hcp_assign    (&self)     -> &    HcpAssign;
    fn get_hcp_assign_mut(&mut self) -> &mut HcpAssign;

    // the global clock mode may not equal to the clock mode of this component
    fn retrieve_clk_mode(&self) -> ClockMode;
    ///fn get_ue_type(&self) -> UeType;
    fn get_des_slice(&self) -> Slice; /// typically it should start from 0
    fn get_priority(&self) -> i32;

    fn do_asm(&self,
              srci       : HcpIdent,
              des_slice  : Option<Slice>,
              src_slice  : Slice,
              arena      : &mut ModelArena) -> AssignMeta;

    fn gen_update_event(&self,
                        srci     : HcpIdent,
                        des_slice: Option<Slice>,
                        src_slice: Slice,
                        arena    : &mut ModelArena,
    ) -> UpdateEventIdent {
        let std_des_slice = self.get_des_slice();
        let my_des_slice = des_slice.as_ref().unwrap_or(&std_des_slice);
        let my_src_slice = src_slice;

        let resolved_des_slice = my_des_slice.get_match_size_sub_slice(&my_src_slice);
        let resolved_src_slice = my_src_slice.get_match_size_sub_slice(my_des_slice);

        arena.make_ue_basic(srci, resolved_des_slice, resolved_src_slice, self.get_priority(), self.retrieve_clk_mode(), false, None)
    }

    fn gen_asm_meta(&self,
                    des_i    : HcpIdent,
                    srci     : HcpIdent,
                    des_slice: Option<Slice>,
                    src_slice: Slice,
                    arena    : &mut ModelArena,
    ) -> AssignMeta {
        let uei = self.gen_update_event(srci, des_slice, src_slice, arena);
        AssignMeta::new(des_i, uei, self.retrieve_clk_mode())
    }


    fn add_update_event(&mut self, event: UpdateEventIdent) {
        self.get_hcp_assign_mut().update_pool.add_update_event(event);
    }

    fn get_update_pool(&self) -> &UpdatePool {
        &self.get_hcp_assign().update_pool
    }

    // Sort the update pool by priority then sub-priority using the arena for key lookup.
    fn sort_events(&mut self, arena: &ModelArena) {
        self.get_hcp_assign_mut().update_pool.sort_events(arena);
    }


}

/*
   Hcp = Hw Component Property
*/

#[derive(Default)]
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