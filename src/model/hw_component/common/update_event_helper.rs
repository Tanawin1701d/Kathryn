use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::slice::Slice;
use crate::model::hw_component::common::update_event::{UeBasic, UeCond, UpdatingEvent};
use crate::model::hw_component::common::update_event_ident::UpdateEventIdent;
use crate::model::hw_component::common::asm_mode::get_asm_pri_val;
use crate::model::model_arena::ModelArena;

pub fn create_ue_helper_basic(
    srci         : HcpIdent,
    des_slice    : Slice,
    src_slice    : Slice,
    priority     : i32,
    cm           : ClockMode,
    auto_priority: bool,
) -> UeBasic {
    let mut event = UeBasic::new(srci, des_slice, src_slice);
    event.set_priority(if auto_priority { get_asm_pri_val() } else { priority });
    event.set_clk_mode(cm);
    event
}

pub fn create_ue_helper_add_dis(
    _cond_i : Option<HcpIdent>,
    _state_i: Option<HcpIdent>,
    _ueb    : UpdateEventIdent,
    _arena  : &mut ModelArena,
) -> UpdateEventIdent {
    // TODO wait for add logic function
    // unimplemented!()
}

pub fn create_ue_helper_full(
    cond          : Option<HcpIdent>,
    state         : Option<HcpIdent>,
    value         : HcpIdent,
    des_slice     : Slice,
    src_slice     : Slice,
    priority      : i32,
    cm            : ClockMode,
    auto_priority : bool,
    model_arena   : &mut ModelArena,
) -> UpdateEventIdent {
    let basic_event = create_ue_helper_basic(value, des_slice, src_slice, priority, cm, auto_priority);

    if cond.is_none() && state.is_none() {
        model_arena.insert_ue_basic(basic_event)
    } else {
        let basic_ident = model_arena.insert_ue_basic(basic_event);
        create_ue_helper_add_dis(cond, state, basic_ident, model_arena)
    }
}

/// Mux helper: selects `left` when `select_left` is true (1-bit), otherwise `right`.
pub fn create_mux_ue_helper(
    left        : UpdateEventIdent,
    right       : UpdateEventIdent,
    select_left : HcpIdent,
    model_arena : &mut ModelArena,
) -> UpdateEventIdent {
    let left_priority  = model_arena.get_ue_common(&left).get_priority();
    let left_clk_mode  = model_arena.get_ue_common(&left).get_clk_mode();
    let mut uec = UeCond::new();
    uec.add_sub_stmt(Some(select_left), left,  left_priority, left_clk_mode);
    uec.add_sub_stmt(None,              right, 0,             ClockMode::ClkUnused);
    model_arena.insert_ue_cond(uec)
}
