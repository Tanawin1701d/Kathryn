

use crate::model::hw_component::common::slice::Slice;
use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HcpIdentifiable};
use crate::model::hw_component::common::update_event::DEFAULT_UE_PRI_INTERNAL_MIN;
use crate::model::hw_component::common::update_event_ident::UpdateEventIdent;
use crate::model::hw_component::common::update_pool::UpdatePool;
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::NcpIdent;
use crate::model::common::identifier::Identifiable;

// HcpIdentifiable supertrait lets the asm helpers read the destination ident
// (always `self`) directly, so callers never pass it in.
pub trait HcpAssignable: HcpIdentifiable {

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
              arena      : &mut ModelArena) -> NcpIdent;

    fn gen_update_event(&self,
                        srci     : HcpIdent,
                        des_slice: Option<Slice>,
                        src_slice: Slice,
                        arena    : &mut ModelArena,
    ) -> UpdateEventIdent {
        self.gen_update_event_with_pri(srci, des_slice, src_slice, self.get_priority(), self.retrieve_clk_mode(), arena)
    }

    // Like `gen_update_event` but with an explicit priority instead of `get_priority`.
    fn gen_update_event_with_pri(&self,
                                 srci     : HcpIdent,
                                 des_slice: Option<Slice>,
                                 src_slice: Slice,
                                 priority : i32,
                                 clk_mode : ClockMode,
                                 arena    : &mut ModelArena,
    ) -> UpdateEventIdent {
        let std_des_slice = self.get_des_slice();
        let my_des_slice = des_slice.as_ref().unwrap_or(&std_des_slice);
        let my_src_slice = src_slice;

        let resolved_des_slice = my_des_slice.get_match_size_sub_slice(&my_src_slice);
        let resolved_src_slice = my_src_slice.get_match_size_sub_slice(my_des_slice);

        arena.make_ue_basic(srci, resolved_des_slice, resolved_src_slice, priority, clk_mode, false, None)
    }

    fn gen_asm_meta(&self,
                    srci     : HcpIdent,
                    des_slice: Option<Slice>,
                    src_slice: Slice,
                    arena    : &mut ModelArena,
    ) -> AssignMeta {
        let uei = self.gen_update_event(srci, des_slice, src_slice, arena);
        AssignMeta::new(self.get_ident(), uei, self.retrieve_clk_mode())
    }

    fn gen_asm_node(&self,
                    srci     : HcpIdent,
                    des_slice: Option<Slice>,
                    src_slice: Slice,
                    arena    : &mut ModelArena,
    ) -> NcpIdent {
        let name = format!("{}_asm", self.get_ident().get_global_name());
        let am   = self.gen_asm_meta(srci, des_slice, src_slice, arena);
        arena.make_asm_node(false, &name, am)   // system-generated assignment node
    }




    fn add_update_event(&mut self, event: UpdateEventIdent) {
        self.get_hcp_assign_mut().update_pool.add_update_event(event);
    }

    // Generate an update event driving this HCP from `srci` and push it into the pool.
    // Bound at the internal-min priority so it always loses to any user assignment.
    fn bind_src(&mut self,
                srci     : HcpIdent,
                des_slice: Option<Slice>,
                src_slice: Slice,
                priority : Option<i32>,
                clk_mode : Option<ClockMode>,
                clk_src  : Option<HcpIdent>,
                arena    : &mut ModelArena,
    ) {
        let clk_mode_upwrap = clk_mode.unwrap_or(ClockMode::ClkFree);
        let is_clk_sen = clk_mode_upwrap == ClockMode::PosEdge || clk_mode_upwrap == ClockMode::NegEdge;

        if is_clk_sen {
            if clk_src.is_none(){
                panic!("no clk_src specified for binded source with posedge or negedge clk");
            }
        }

        let ue_i = self.gen_update_event_with_pri(srci, des_slice, src_slice,
                                                  priority.unwrap_or(DEFAULT_UE_PRI_INTERNAL_MIN),
                                                  clk_mode_upwrap,
                                                  arena);

        if is_clk_sen {
            let mut ue = arena.take_ue(ue_i);
            ue.set_clk_src_i(clk_src);
            arena.replace_back_ue(ue);
        }

        self.add_update_event(ue_i);
    }

    fn get_update_pool(&self) -> &UpdatePool {
        &self.get_hcp_assign().update_pool
    }

    // Sort the update pool by priority then sub-priority using the arena for key lookup.
    fn sort_events(&mut self, arena: &ModelArena) {
        self.get_hcp_assign_mut().update_pool.sort_events(arena);
    }

    // ---- IO mark (delegates to the stored HcpAssign) ----
    fn mark_as_io      (&mut self, is_input: bool, io_name: String) { self.get_hcp_assign_mut().mark_as_io(is_input, io_name); }
    fn is_marked_as_io (&self) -> bool                              { self.get_hcp_assign().is_marked_as_io() }
    fn get_io_mark     (&self) -> Option<&HcpIoMark>                { self.get_hcp_assign().get_io_mark() }

}

/*
   Hcp = Hw Component Property
*/

#[derive(Default)]
pub struct HcpAssign{
    update_pool : UpdatePool,
    io_mark     : Option<HcpIoMark>,
}

impl HcpAssign {
    pub fn new() -> HcpAssign {
        HcpAssign {
            update_pool : UpdatePool::new(),
            io_mark     : None,
        }
    }
    // ---- IO mark ----
    pub fn mark_as_io      (&mut self, is_input: bool, io_name: String) { self.io_mark = Some(HcpIoMark { is_input, io_name }); }
    pub fn is_marked_as_io (&self) -> bool                              { self.io_mark.is_some() }
    pub fn get_io_mark     (&self) -> Option<&HcpIoMark>                { self.io_mark.as_ref() }
}

// IO designation stamped onto an HcpAssign: the user-facing port name and its
// direction. `None` on HcpAssign means the component is not an IO port.
#[derive(Clone)]
pub struct HcpIoMark {
    is_input : bool,
    io_name  : String,
}

impl HcpIoMark {
    pub fn is_input(&self) -> bool  { self.is_input }
    pub fn io_name (&self) -> &str  { &self.io_name }
}