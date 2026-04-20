use std::sync::atomic::{AtomicU64, Ordering};
use crate::model::hw_component::common::hcp_assign::HcpAssignable;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::update_event::{UpdatingEvent, UeBasic};
use crate::model::hw_component::common::update_event_helper::{create_ue_helper_add_dis, create_mux_ue_helper};

static ASSIGN_CNT: AtomicU64 = AtomicU64::new(0);

pub struct AssignMeta {
    target_hwc       : HcpIdent,
    input_event      : Option<UeBasic>,
    pre_update_event : Box<dyn UpdatingEvent>,
    finished         : bool,
}

impl AssignMeta {
    pub fn new(target_hwc: HcpIdent, mut input_element: UeBasic) -> Self {
        let cnt = ASSIGN_CNT.fetch_add(1, Ordering::Relaxed);
        input_element.set_sub_priority(cnt);
        let pre: Box<dyn UpdatingEvent> = Box::new(input_element.clone());
        Self {
            target_hwc,
            input_event     : Some(input_element),
            pre_update_event: pre,
            finished        : false,
        }
    }

    pub fn new_complex(
        target_hwc      : HcpIdent,
        pre_update_event: Box<dyn UpdatingEvent>,
        cur_assign_cnt  : u64,
    ) -> Self {
        let mut event = pre_update_event;
        event.set_sub_priority(cur_assign_cnt);
        Self {
            target_hwc,
            input_event     : None,
            pre_update_event: event,
            finished        : false,
        }
    }

    pub fn is_joinable(&self, rhs: &AssignMeta) -> bool {
        self.target_hwc == rhs.target_hwc &&
        self.pre_update_event.is_joinable(&*rhs.pre_update_event)
    }

    pub fn final_update(self, model_arena: &mut ModelArena) {
        let target = model_arena.borrow_asb_mut(self.target_hwc);
        target.add_update_event(self.pre_update_event);
    }

    pub fn get_target_hw      (&self) -> &HcpIdent { &self.target_hwc }
    pub fn get_input_event_mut(&mut self) -> &mut Option<UeBasic> { &mut self.input_event }
    pub fn get_cur_assign_cnt (&self) -> u64       { self.pre_update_event.get_sub_priority() }

    pub fn set_new_editing_event(&mut self, event: Box<dyn UpdatingEvent>) {
        self.pre_update_event = event;
    }

    pub fn add_specific_pre_condition(&mut self, cond: HcpIdent) {
        let old = self.pre_update_event.clone_box();
        self.pre_update_event = create_ue_helper_add_dis(Some(cond), None, old);
    }

    pub fn mux(&mut self, right: &mut AssignMeta, select_left: HcpIdent) -> AssignMeta {
        let mux_event = create_mux_ue_helper(
            self.pre_update_event.clone_box(),
            right.pre_update_event.clone_box(),
            select_left,
        );

        self.set_finished();
        right.set_finished();

        AssignMeta::new_complex(self.target_hwc.clone(), mux_event, self.get_cur_assign_cnt())
    }

    pub fn is_finished(&self) -> bool { self.finished }
    pub fn set_finished(&mut self) { self.finished = true; }
}
