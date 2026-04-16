use std::rc::Rc;
use std::cell::RefCell;
use std::sync::atomic::{AtomicU64, Ordering};
use crate::common::obj::SPTR;
use crate::model::hw_component::common::hcp_assign::Assignable;
use crate::model::hw_component::common::hcp_read::Readable;
use crate::model::hw_component::common::update_event::{UpdatingEvent, UeBasic};
use crate::model::hw_component::common::update_event_helper::{create_ue_helper_add_dis, create_mux_ue_helper};

static ASSIGN_CNT: AtomicU64 = AtomicU64::new(0);

pub struct AssignMeta {
    target_hwc         : SPTR<dyn Assignable>,
    input_event        : Option<SPTR<UeBasic>>,
    pre_update_event   : SPTR<dyn UpdatingEvent>,
}

impl AssignMeta {
    /// Primary constructor: takes a basic event, auto-increments ASSIGN_CNT.
    pub fn new(
        target_hwc    : SPTR<dyn Assignable>,
        input_element : SPTR<UeBasic>,
    ) -> Self {
        let cnt = ASSIGN_CNT.fetch_add(1, Ordering::Relaxed);

        input_element.borrow_mut().set_sub_priority(cnt);

        // 👇 Coerce HERE (at the outer Rc level)
        let pre_update_element: SPTR<dyn UpdatingEvent> = input_element.clone();

        Self {
            target_hwc,
            input_event     : Some(input_element),
            pre_update_event: pre_update_element,
        }
    }

    /// Secondary constructor: complex event with an explicit assign count, no input_element.
    pub fn new_complex(
        target_hwc      : SPTR<dyn Assignable>,
        pre_update_event: SPTR<dyn UpdatingEvent>,
        cur_assign_cnt: u64,
    ) -> Self {
        pre_update_event.borrow_mut().set_sub_priority(cur_assign_cnt);
        Self {
            target_hwc,
            input_event: None,
            pre_update_event,
        }
    }

    pub fn is_joinable(&self, rhs: &AssignMeta) -> bool {
        Rc::ptr_eq(&self.target_hwc, &rhs.target_hwc) &&
        self.pre_update_event.borrow().is_joinable(&*rhs.pre_update_event.borrow())
    }

    /// Moves pre_update_element into the pool.
    pub fn final_update(&mut self) {
            self.target_hwc.borrow_mut().
                add_update_event(self.pre_update_event.clone());

    }

    pub fn get_target_hw     (&self) -> SPTR<dyn Assignable> {self.target_hwc.clone()}
    pub fn get_cur_assign_cnt(&self) -> u64 {
        self.pre_update_event.borrow().get_sub_priority()
    }

    pub fn set_new_editing_event(&mut self, event: SPTR<dyn UpdatingEvent>) {
        self.pre_update_event = event;
    }

    pub fn add_specific_pre_condition(&mut self, cond: SPTR<dyn Readable>) {
        self.pre_update_event =  create_ue_helper_add_dis(Some(cond), None, self.pre_update_event.clone());
    }

    pub fn mux(&self, right: &AssignMeta, select_left: SPTR<dyn Readable>) -> AssignMeta {
        let mux_event = create_mux_ue_helper(
            self.pre_update_event.clone(),
            right.pre_update_event.clone(),
            select_left,
        );
        let cnt = self.get_cur_assign_cnt();
        AssignMeta::new_complex(self.target_hwc.clone(), mux_event, cnt)
    }
}