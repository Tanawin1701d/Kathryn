use std::rc::Rc;
use std::cell::RefCell;
use std::sync::atomic::{AtomicU64, Ordering};
use crate::model::hw_component::common::operable::Operable;
use crate::model::hw_component::common::update_event::UpdatingEvent;
use crate::model::hw_component::common::update_pool::UpdatePool;

static ASSIGN_CNT: AtomicU64 = AtomicU64::new(0);

pub struct AssignMeta {
    event_pool         : Rc<RefCell<UpdatePool>>,
    pre_update_element : Option<Box<dyn UpdatingEvent>>,
}

impl AssignMeta {
    /// Primary constructor: takes a basic event, auto-increments ASSIGN_CNT.
    pub fn new(
        event_pool    : Rc<RefCell<UpdatePool>>,
        input_element : Box<dyn UpdatingEvent>,
    ) -> Self {
        let cnt = ASSIGN_CNT.fetch_add(1, Ordering::Relaxed);
        let mut input_element = input_element;
        input_element.set_sub_priority(cnt);
        Self {
            event_pool,
            pre_update_element : Some(input_element),
        }
    }

    /// Secondary constructor: complex event with an explicit assign count, no input_element.
    pub fn new_complex(
        event_pool       : Rc<RefCell<UpdatePool>>,
        pre_update_event : Box<dyn UpdatingEvent>,
        cur_assign_cnt   : u64,
    ) -> Self {
        let mut pre_update_event = pre_update_event;
        pre_update_event.set_sub_priority(cur_assign_cnt);
        Self {
            event_pool,
            pre_update_element : Some(pre_update_event),
        }
    }

    pub fn is_joinable(&self, rhs: &AssignMeta) -> bool {
        Rc::ptr_eq(&self.event_pool, &rhs.event_pool) &&
        self.pre_update_element.as_ref().unwrap()
            .is_joinable(&**rhs.pre_update_element.as_ref().unwrap())
    }

    /// Moves pre_update_element into the pool.
    pub fn final_update(&mut self) {
        if let Some(event) = self.pre_update_element.take() {
            self.event_pool.borrow_mut().add_update_event(event);
        }
    }

    pub fn get_event_pool    (&self) -> Rc<RefCell<UpdatePool>> { self.event_pool.clone() }
    pub fn get_cur_assign_cnt(&self) -> u64 {
        self.pre_update_element.as_ref().unwrap().get_sub_priority()
    }

    pub fn set_new_editing_event(&mut self, event: Box<dyn UpdatingEvent>) {
        self.pre_update_element = Some(event);
    }

    pub fn add_specific_pre_condition(&mut self, _cond: Rc<dyn Operable>) {
        todo!("createUEHelper not yet implemented")
    }

    pub fn mux(&self, _right: &AssignMeta, _select_left: Rc<dyn Operable>) -> AssignMeta {
        todo!("createMuxUEHelper not yet implemented")
    }
}