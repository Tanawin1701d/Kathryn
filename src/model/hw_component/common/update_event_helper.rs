use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::slice::Slice;
use crate::model::hw_component::common::update_event::{UeBasic, UeCond, UpdatingEvent};
use crate::model::hw_component::common::asm_mode::get_asm_pri_val;

pub fn create_ue_helper_basic(
    value        : HcpIdent,
    sl           : Slice,
    priority     : i32,
    cm           : ClockMode,
    auto_priority: bool,
) -> UeBasic {
    let mut event = UeBasic::new(value, sl);
    event.set_priority(if auto_priority { get_asm_pri_val() } else { priority });
    event.set_clk_mode(cm);
    event
}

pub fn create_ue_helper_add_dis(
    cond : Option<HcpIdent>,
    state: Option<HcpIdent>,
    ueb  : Box<dyn UpdatingEvent>
) -> Box<dyn UpdatingEvent> {
    // TODO wait for add logic function
    unimplemented!()
}

pub fn create_ue_helper_full(
    cond          : Option<HcpIdent>,
    state         : Option<HcpIdent>,
    value         : HcpIdent,
    sl            : Slice,
    priority      : i32,
    cm            : ClockMode,
    auto_priority : bool
) -> Box<dyn UpdatingEvent> {
    let basic_event = create_ue_helper_basic(value, sl, priority, cm, auto_priority);

    if cond.is_none() && state.is_none() {
        return Box::new(basic_event);
    }

    create_ue_helper_add_dis(cond, state, Box::new(basic_event))
}

/// Mux helper: selects `left` when `select_left` is true (1-bit), otherwise `right`.
pub fn create_mux_ue_helper(
    left        : Box<dyn UpdatingEvent>,
    right       : Box<dyn UpdatingEvent>,
    select_left : HcpIdent,
) -> Box<dyn UpdatingEvent> {
    let mut uec = UeCond::new();
    uec.add_sub_stmt(Some(select_left), left);
    uec.add_sub_stmt(None, right);
    Box::new(uec)
}
