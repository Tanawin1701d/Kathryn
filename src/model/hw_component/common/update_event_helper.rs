use std::cell::RefCell;
use std::rc::Rc;
use crate::common::obj::SPTR;
use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::hcp_read::Readable;
use crate::model::hw_component::common::slice::Slice;
use crate::model::hw_component::common::update_event::{UeBasic, UpdatingEvent};
use crate::model::hw_component::common::asm_mode::get_asm_pri_val;

pub fn create_ue_helper_basic(
    value       : SPTR<dyn Readable>,
    sl          : Slice,
    priority    : i32,
    cm          : ClockMode,
    auto_priority: bool,
) -> SPTR<UeBasic> {

    let mut event = UeBasic::new(value, sl);
    event.set_priority(priority);
    event.set_clk_mode(cm);

    if auto_priority {
        // override priority if auto mode is required
        event.set_priority(get_asm_pri_val());
    }

    Rc::new(RefCell::new(event))
}

pub fn create_ue_helper_add_dis(
    cond : Option<SPTR<dyn Readable>>,
    state: Option<SPTR<dyn Readable>>,
    ueb  : SPTR<dyn UpdatingEvent>
) -> SPTR<dyn UpdatingEvent> {
    // TODO wait for add logic function
}

pub fn create_ue_helper_full(
    cond          : Option<SPTR<dyn Readable>>,
    state         : Option<SPTR<dyn Readable>>,
    value         : SPTR<dyn Readable>,
    sl            : Slice,
    priority      : i32,
    cm            : ClockMode,
    auto_priority : bool
){
    if cond.is_none() && state.is_none() {
        return;
    }

    let basic_ue = create_ue_helper_basic(value, sl, priority, cm, auto_priority);
    let add_ue   = create_ue_helper_add_dis(cond, state, basic_ue);



}


