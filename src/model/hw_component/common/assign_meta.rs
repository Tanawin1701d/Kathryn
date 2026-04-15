use std::rc::Rc;
use std::cell::RefCell;
use std::sync::atomic::{AtomicU64, Ordering};
use crate::common::obj::SPTR;
use crate::model::hw_component::common::hcp_assign::Assignable;
use crate::model::hw_component::common::hcp_read::Readable;
use crate::model::hw_component::common::update_event::{UpdatingEvent, UeBasic};

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
        let pre_update_element: SPTR<dyn UpdatingEvent>> =
            input_element.clone();

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

    pub fn add_specific_pre_condition(&mut self, _cond: Rc<dyn Readable>) {
        todo!("createUEHelper not yet implemented")
    }

    pub fn mux(&self, _right: &AssignMeta, _select_left: Rc<dyn Readable>) -> AssignMeta {
        todo!("createMuxUEHelper not yet implemented")
    }
}