use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::hcp_assign::HcpAssignable;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::update_event_ident::UpdateEventIdent;
use crate::model::model_arena::ModelArena;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct AssignMeta {
    target_hwc      : HcpIdent,
    clk_mode        : ClockMode,
    input_event_i   : Option<UpdateEventIdent>, // the basicUpdateEvent of the target HWC, if tehre is complex assignment if () {...} this value should be None
    pre_update_event: UpdateEventIdent,         // the latest update event that has been assigned to the target HWC
    finished        : bool,
}

impl AssignMeta {
    pub fn new(target_hwc: HcpIdent, ue: UpdateEventIdent, clk_mode: ClockMode) -> Self {
        Self {
            target_hwc,
            clk_mode,
            input_event_i   : Some(ue),
            pre_update_event: ue,
            finished        : false,
        }
    }

    pub fn final_update(self, model_arena: &mut ModelArena) {
        let target = model_arena.borrow_asb_mut(self.target_hwc);
        target.add_update_event(self.pre_update_event);
    }

    pub fn get_target_hw        (&self)     -> HcpIdent                       { self.target_hwc         }
    pub fn get_clk_mode         (&self)     -> ClockMode                      { self.clk_mode           }
    pub fn get_input_event_i    (&self)     -> Option<UpdateEventIdent>       { self.input_event_i      }
    pub fn get_input_event_i_mut(&mut self) -> &mut Option<UpdateEventIdent>  { &mut self.input_event_i }
    pub fn get_pre_update_event (&self)     -> UpdateEventIdent               { self.pre_update_event   }

    pub fn set_pre_update_event(&mut self, event: UpdateEventIdent) {
        self.pre_update_event = event;
    }

    pub fn add_specific_pre_condition(&mut self, cond: HcpIdent, arena: &mut ModelArena) {
        self.pre_update_event = arena.make_ue_add_dis(Some(cond), None, self.pre_update_event);
    }

    pub fn mux(&mut self, right: &mut AssignMeta, select_left: HcpIdent, arena: &mut ModelArena) -> AssignMeta {
        let mux_event = arena.make_ue_mux(self.pre_update_event, right.pre_update_event, select_left);
        self.set_finished();
        right.set_finished();
        AssignMeta::new(self.target_hwc, mux_event, self.clk_mode)
    }

    pub fn is_finished (&self)     -> bool { self.finished      }
    pub fn set_finished(&mut self)         { self.finished = true; }
}
