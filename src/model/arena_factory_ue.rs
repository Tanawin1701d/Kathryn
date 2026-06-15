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
pub fn assert_clk_src_consistent(cm: ClockMode, clk_src: Option<HcpIdent>, lazy_edge_clk: bool) {
    if cm == ClockMode::ClkFree && clk_src.is_some() {
        panic!("make_ue_*: clk_src must be None when clk_mode is ClkFree");
    }

    if (cm == ClockMode::PosEdge || cm == ClockMode::NegEdge) &&
       (!lazy_edge_clk) && (!clk_src.is_some()) {
        panic!("make_ue_*: clk_src must be assigned when clk_mode is Posedge or NegEdge");
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
        /// check constrain
        assert_clk_src_consistent(cm, clk_src, true);   // for clk, it may right now not ready for clk signal
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
                let expr = self.make_expression(false, "cond_dis_expr", LogicOp::BitwiseAnd,
                                                cond, state,
                                                Some(Slice::new(0, 1)),
                                                Some(Slice::new(0, 1)));   // it is lock to (0, 1)
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
        assert_clk_src_consistent(clk_mode, clk_src, true);
        let mut uec = UeCond::new();
        uec.add_sub_stmt(cond_hcp, Some(_ueb_i), priority, clk_mode, clk_src);
        self.insert_ue_cond(uec)
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
        assert_clk_src_consistent(cm, clk_src, true);
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
        let left_priority  = self.get_ue_common(&left ).get_priority();
        let right_priority = self.get_ue_common(&right).get_priority();
        let left_clk_mode  = self.get_ue_common(&left ).get_clk_mode();
        let right_clk_mode = self.get_ue_common(&right).get_clk_mode();
        // mux operands must be combinational — a clocked operand would make the wrapping
        // UeCond carry an ambiguous clock domain.
        assert_eq!(left_clk_mode,  ClockMode::ClkUnused, "make_ue_mux: left  operand must be ClkUnused");
        assert_eq!(right_clk_mode, ClockMode::ClkUnused, "make_ue_mux: right operand must be ClkUnused");
        // belt-and-braces: both operands must share the same clock mode and priority so
        // the mux is unambiguously single-domain and joinable with sibling UEs.
        assert_eq!(left_priority,  right_priority,  "make_ue_mux: left/right operands must share priority");

        let mut uec = UeCond::new();
        uec.add_sub_stmt(Some(select_left), Some(left),  left_priority, ClockMode::ClkUnused, None);
        uec.add_sub_stmt(None,              Some(right), left_priority, ClockMode::ClkUnused, None);
        self.insert_ue_cond(uec)
    }
}
