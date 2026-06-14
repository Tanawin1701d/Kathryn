use crate::model::arena_factory_ue::assert_clk_src_consistent;
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
}

impl AssignMeta {
    pub fn new(target_hwc: HcpIdent, ue: UpdateEventIdent, clk_mode: ClockMode) -> Self {
        Self {
            target_hwc,
            clk_mode,
            input_event_i   : Some(ue),
            pre_update_event: ue,
        }
    }

    pub fn final_update(self, model_arena: &mut ModelArena) {
        let mut hcp = model_arena.take_hcp(self.target_hwc);
        hcp.add_update_event(self.pre_update_event);
        model_arena.replace_back_hcp(hcp);
    }

    pub fn get_target_hw        (&self)     -> HcpIdent                       { self.target_hwc         }
    pub fn get_clk_mode         (&self)     -> ClockMode                      { self.clk_mode           }
    pub fn get_input_event_i    (&self)     -> Option<UpdateEventIdent>       { self.input_event_i      }
    pub fn get_input_event_i_mut(&mut self) -> &mut Option<UpdateEventIdent>  { &mut self.input_event_i }
    pub fn get_pre_update_event (&self)     -> UpdateEventIdent               { self.pre_update_event   }

    pub fn add_specific_pre_condition(&mut self, cond: HcpIdent, arena: &mut ModelArena) {
        let cm      = arena.get_ue_common(&self.pre_update_event).get_clk_mode();
        let clk_src = arena.get_ue_common(&self.pre_update_event).get_clk_src_i();
        assert_clk_src_consistent(cm, clk_src, false);
        self.pre_update_event = arena.make_ue_add_dis(Some(cond), None, self.pre_update_event);
    }

    /// Set the clock source on `input_event_i`.  No-op if input_event_i is None
    /// (complex-assignment path where no single basic UE was recorded).
    /// Panics via assert_clk_src_consistent if clk_src conflicts with the UE's clock mode.
    pub fn try_set_clk_src(&mut self, clk_src: HcpIdent, arena: &mut ModelArena) {

        if !matches!(self.clk_mode, ClockMode::PosEdge | ClockMode::NegEdge) { return; }
        if let Some(ue_i) = self.input_event_i {
            let mut ue = arena.take_ue(ue_i);
            ue.set_clk_src_i(Some(clk_src));
            arena.replace_back_ue(ue);
        }
    }

    pub fn mux(&mut self, right: &mut AssignMeta, select_left: HcpIdent, arena: &mut ModelArena) -> AssignMeta {
        let mux_event = arena.make_ue_mux(self.pre_update_event, right.pre_update_event, select_left);
        AssignMeta::new(self.target_hwc, mux_event, self.clk_mode)
    }
}
