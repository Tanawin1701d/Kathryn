use std::cell::RefCell;
use std::rc::Rc;
use crate::common::obj::SPTR;
use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::hcp_read::Readable;
use crate::model::hw_component::common::slice::Slice;
use crate::model::hw_component::common::update_event::{UeBasic, UeCond, UpdatingEvent};
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
) -> SPTR<dyn UpdatingEvent> {

    let basic_event = create_ue_helper_basic(value,sl,priority,cm,auto_priority);

    if cond.is_none() && state.is_none() {
        return basic_event;
    }

    let add_ue   = create_ue_helper_add_dis(cond, state, basic_event);

    add_ue
}

/// Mux helper: selects `left` when `select_left` is true (1-bit), otherwise `right`.
/// Equivalent to C++ createMuxUEHelper.
pub fn create_mux_ue_helper(
    left        : SPTR<dyn UpdatingEvent>,
    right       : SPTR<dyn UpdatingEvent>,
    select_left : SPTR<dyn Readable>,
) -> SPTR<dyn UpdatingEvent> {
    // assert: select_left must be a 1-bit signal
    // (full check requires Readable::get_slice — assert added when available)
    let mut uec = UeCond::new();
    uec.add_sub_stmt(Some(select_left), left);
    uec.add_sub_stmt(None, right);
    Rc::new(RefCell::new(uec))
}
