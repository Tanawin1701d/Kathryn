use std::rc::Rc;
use std::cell::RefCell;
use std::sync::atomic::{AtomicU64, Ordering};
use crate::model::hw_component::common::operable::Operable;
use crate::model::hw_component::common::update_event::UpdatingEvent;
use crate::model::hw_component::common::update_pool::UpdatePool;

static ASSIGN_CNT: AtomicU64 = AtomicU64::new(0);

pub struct AssignMeta {
    event_pool         : Rc<RefCell<UpdatePool>>,
    input_element      : Option<Rc<RefCell<dyn UpdatingEvent>>>,
    pre_update_element : Rc<RefCell<dyn UpdatingEvent>>,
}

impl AssignMeta {
    /// Primary constructor: takes a basic event, auto-increments ASSIGN_CNT.
    pub fn new(
        event_pool    : Rc<RefCell<UpdatePool>>,
        input_element : Rc<RefCell<dyn UpdatingEvent>>,
    ) -> Self {
        let cnt = ASSIGN_CNT.fetch_add(1, Ordering::Relaxed);
        input_element.borrow_mut().set_sub_priority(cnt);
        Self {
            event_pool,
            input_element      : Some(input_element.clone()),
            pre_update_element : input_element,
        }
    }

    /// Secondary constructor: complex event with an explicit assign count, no input_element.
    pub fn new_complex(
        event_pool       : Rc<RefCell<UpdatePool>>,
        pre_update_event : Rc<RefCell<dyn UpdatingEvent>>,
        cur_assign_cnt   : u64,
    ) -> Self {
        pre_update_event.borrow_mut().set_sub_priority(cur_assign_cnt);
        Self {
            event_pool,
            input_element      : None,
            pre_update_element : pre_update_event,
        }
    }

    pub fn is_joinable(&self, rhs: &AssignMeta) -> bool {
        Rc::ptr_eq(&self.event_pool, &rhs.event_pool) &&
        self.pre_update_element.borrow()
            .is_joinable(&*rhs.pre_update_element.borrow())
    }

    /// Pushes pre_update_element into the pool.
    pub fn final_update(&self) {
        self.event_pool.borrow_mut()
            .add_update_event(self.pre_update_element.clone());
    }

    pub fn get_event_pool    (&self) -> Rc<RefCell<UpdatePool>>          { self.event_pool.clone()          }
    pub fn get_current_event (&self) -> Rc<RefCell<dyn UpdatingEvent>>   { self.pre_update_element.clone()  }
    pub fn get_cur_assign_cnt(&self) -> u64 { self.pre_update_element.borrow().get_sub_priority() }

    pub fn set_new_editing_event(&mut self, event: Rc<RefCell<dyn UpdatingEvent>>) {
        self.pre_update_element = event;
    }

    pub fn add_specific_pre_condition(&mut self, _cond: Rc<dyn Operable>) {
        todo!("createUEHelper not yet implemented")
    }

    pub fn mux(&self, _right: &AssignMeta, _select_left: Rc<dyn Operable>) -> AssignMeta {
        todo!("createMuxUEHelper not yet implemented")
    }
}
