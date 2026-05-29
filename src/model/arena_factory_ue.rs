use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::asm_mode::get_asm_pri_val;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::hw_component::common::slice::Slice;
use crate::model::hw_component::common::update_event::{UeBasic, UeCond, UpdatingEvent};
use crate::model::hw_component::common::update_event_ident::UpdateEventIdent;
use crate::model::model_arena::ModelArena;

// Combinational UEs (ClkFree) must not carry a clock source — that combination is meaningless
// and would make is_joinable() produce false positives across unrelated clock domains.
fn assert_clk_src_consistent(cm: ClockMode, clk_src: Option<HcpIdent>) {
    if cm == ClockMode::ClkFree && clk_src.is_some() {
        panic!("make_ue_*: clk_src must be None when clk_mode is ClkFree");
    }
}

impl ModelArena {
    pub fn make_ue_basic(
        &mut self,
        srci         : HcpIdent,
        des_slice    : Slice,
        src_slice    : Slice,
        priority     : i32,
        cm           : ClockMode,
        auto_priority: bool,
        clk_src      : Option<HcpIdent>,
    ) -> UpdateEventIdent {
        assert_clk_src_consistent(cm, clk_src);
        let mut event = UeBasic::new(srci, des_slice, src_slice);
        event.set_priority(if auto_priority { get_asm_pri_val() } else { priority });
        event.set_clk_mode(cm);
        event.set_clk_src_i(clk_src);
        self.insert_ue_basic(event)
    }

    pub fn make_ue_add_dis(
        &mut self,
        _cond_i : Option<HcpIdent>,
        _state_i: Option<HcpIdent>,
        _ueb_i  : UpdateEventIdent,
    ) -> UpdateEventIdent {
        let cond_hcp = match (_cond_i, _state_i) {
            (Some(cond), Some(state)) => {
                let expr = self.make_expression(false, "cond_dis_expr", LogicOp::BitwiseAnd, cond, state, None, None);
                Some(expr)
            },
            (Some(cond),  None      ) => Some(cond),
            (None,        Some(state)) => Some(state),
            (None,        None       ) => unreachable!(),
        };
        // Inherit priority/clk_mode/clk_src from the wrapped UE so the resulting UeCond
        // stays joinable with its sibling UEs in the same update pool.
        let priority = self.get_ue_common(&_ueb_i).get_priority();
        let clk_mode = self.get_ue_common(&_ueb_i).get_clk_mode();
        let clk_src  = self.get_ue_common(&_ueb_i).get_clk_src_i();
        assert_clk_src_consistent(clk_mode, clk_src);
        let mut uec = UeCond::new();
        uec.add_sub_stmt(cond_hcp, Some(_ueb_i), priority, clk_mode, clk_src);
        let cond_i = self.insert_ue_cond(uec);
        // add_sub_stmt initialises clk_src_i to None via its init_meta path;
        // set it after insertion so the UeCond's clk_src matches the wrapped UE's.
        let mut cond_v = self.take_ue_cond(cond_i);
        cond_v.set_clk_src_i(clk_src);
        self.replace_back_ue_cond(cond_v);
        cond_i
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
        clk_src      : Option<HcpIdent>,
    ) -> UpdateEventIdent {
        assert_clk_src_consistent(cm, clk_src);
        let basic_ident = self.make_ue_basic(value, des_slice, src_slice, priority, cm, auto_priority, clk_src);
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
        uec.add_sub_stmt(Some(select_left), Some(left),  left_priority, left_clk_mode, None);
        uec.add_sub_stmt(None,              Some(right), 0,             ClockMode::ClkUnused, None);
        self.insert_ue_cond(uec)
    }
}
