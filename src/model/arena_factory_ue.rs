use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::asm_mode::get_asm_pri_val;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::slice::Slice;
use crate::model::hw_component::common::update_event::{UeBasic, UeCond, UpdatingEvent};
use crate::model::hw_component::common::update_event_ident::UpdateEventIdent;
use crate::model::model_arena::ModelArena;

impl ModelArena {
    pub fn make_ue_basic(
        &mut self,
        srci         : HcpIdent,
        des_slice    : Slice,
        src_slice    : Slice,
        priority     : i32,
        cm           : ClockMode,
        auto_priority: bool,
    ) -> UpdateEventIdent {
        let mut event = UeBasic::new(srci, des_slice, src_slice);
        event.set_priority(if auto_priority { get_asm_pri_val() } else { priority });
        event.set_clk_mode(cm);
        self.insert_ue_basic(event)
    }

    pub fn make_ue_add_dis(
        &mut self,
        _cond_i : Option<HcpIdent>,
        _state_i: Option<HcpIdent>,
        _ueb    : UpdateEventIdent,
    ) -> UpdateEventIdent {
        // TODO wait for add logic function
        unimplemented!()
    }

    pub fn make_ue_full(
        &mut self,
        cond         : Option<HcpIdent>,
        state        : Option<HcpIdent>,
        value        : HcpIdent,
        des_slice    : Slice,
        src_slice    : Slice,
        priority     : i32,
        cm           : ClockMode,
        auto_priority: bool,
    ) -> UpdateEventIdent {
        let basic_ident = self.make_ue_basic(value, des_slice, src_slice, priority, cm, auto_priority);
        if cond.is_none() && state.is_none() {
            basic_ident
        } else {
            self.make_ue_add_dis(cond, state, basic_ident)
        }
    }

    pub fn make_ue_mux(
        &mut self,
        left        : UpdateEventIdent,
        right       : UpdateEventIdent,
        select_left : HcpIdent,
    ) -> UpdateEventIdent {
        let left_priority = self.get_ue_common(&left).get_priority();
        let left_clk_mode = self.get_ue_common(&left).get_clk_mode();
        let mut uec = UeCond::new();
        uec.add_sub_stmt(Some(select_left), left,  left_priority, left_clk_mode);
        uec.add_sub_stmt(None,              right, 0,             ClockMode::ClkUnused);
        self.insert_ue_cond(uec)
    }
}
