use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::hcp_base::HcpBase;
use crate::model::hw_component::common::hcp_assign::{HcpAssign, HcpAssignable};
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HcpIdentifiable, HwComponentType};
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::hw_component::common::slice::Slice;
use crate::model::hw_component::common::update_event::{DEFAULT_UE_PRI_INTERNAL_MIN, DEFAULT_UE_PRI_RST};
use crate::model::hw_component::sp_reg::trigger_sig::{HasTriggerSig, TriggerSig};
use crate::model::model_arena::ModelArena;
use crate::model::common::identifier::{IdentBase, Identifiable};

const DEFAULT_UE_PRI_SY_UNSET    : i32 = DEFAULT_UE_PRI_INTERNAL_MIN;
const DEFAULT_UE_PRI_SY_ACTIVATE : i32 = DEFAULT_UE_PRI_INTERNAL_MIN + 1;
const DEFAULT_UE_PRI_SY_HOLD     : i32 = DEFAULT_UE_PRI_INTERNAL_MIN + 2;
const DEFAULT_UE_PRI_SY_RST      : i32 = DEFAULT_UE_PRI_INTERNAL_MIN + 3;
// INTERRUPT is not supported, so we use the same priority as RST
const DEFAULT_UE_PRI_SY_MRST     : i32 = DEFAULT_UE_PRI_RST;

/// n-bit synchronisation register: raised bit-by-bit as dependent states
pub struct SyncReg {
    assign            : HcpAssign,
    ident             : HcpIdent,
    bit_width         : i32,
    triggers          : TriggerSig,
    // support signals (populated by build_support_signal)
    up_state_i        : Option<HcpIdent>,  // 1-bit Val = 1
    up_full_state_i   : Option<HcpIdent>,  // n-bit Val = all 1s
    down_full_state_i : Option<HcpIdent>,  // n-bit Val = all 0s
    test_wire_i       : Option<HcpIdent>,  // n-bit Wire (observes activations without endExpr gate)
    end_expr_i        : Option<HcpIdent>,  // (self | testWire) == upFullState
    end_expr_inv_i    : Option<HcpIdent>,  // ~endExpr
}

impl Default for SyncReg {
    fn default() -> Self {
        Self {
            assign            : HcpAssign::new(),
            ident             : HcpIdent::default(),
            bit_width         : 1,
            triggers          : TriggerSig::new(),
            up_state_i        : None,
            up_full_state_i   : None,
            down_full_state_i : None,
            test_wire_i       : None,
            end_expr_i        : None,
            end_expr_inv_i    : None,
        }
    }
}

impl SyncReg {
    pub fn new(is_user_com: bool, name: &str, size: i32) -> Self {
        assert!(size > 0, "SyncReg size must be positive");
        Self {
            assign            : HcpAssign::new(),
            ident             : HcpIdent::new(HwComponentType::SyncReg, is_user_com, name),
            bit_width         : size,
            triggers          : TriggerSig::new(),
            up_state_i        : None,
            up_full_state_i   : None,
            down_full_state_i : None,
            test_wire_i       : None,
            end_expr_i        : None,
            end_expr_inv_i    : None,
        }
    }

    pub fn mk(name: &str, size: i32) -> Self { Self::new(false, name, size) }

    pub fn get_ident       (&self)     -> HcpIdent        { self.ident             }
    pub fn get_ident_mut   (&mut self) -> &mut HcpIdent  { &mut self.ident        }
    pub fn get_end_expr_i  (&self) -> Option<HcpIdent> { self.end_expr_i        }

    /// Creates all internal support signals in the arena.
    /// Must be called before `build_update_event`.
    pub fn build_support_signal(&mut self, model_ar: &mut ModelArena) {
        let name = self.ident.get_ident_base().get_name().to_string();
        let size = self.bit_width;

        let up_state_i = model_ar.make_val(false, &format!("{}_UP_STATE",   name), 1, 1);
        self.up_state_i = Some(up_state_i);
        // right now, we only support 64-bit registers, so we can use a single Val for both
        let up_full_val = if size < 64 { (1u64 << size) - 1 } else { u64::MAX };
        let up_full_state_i = model_ar.make_val(false, &format!("{}_UP_FULL",   name), size, up_full_val);
        self.up_full_state_i = Some(up_full_state_i);

        let down_full_state_i = model_ar.make_val(false, &format!("{}_DOWN_FULL", name), size, 0);
        self.down_full_state_i = Some(down_full_state_i);

        let test_wire_i = model_ar.make_wire(false, &format!("{}_TEST_WIRE", name), size);
        self.test_wire_i = Some(test_wire_i);

        // (self | testWire)
        let or_expr = model_ar.make_expression(
            false, &format!("{}_OR_TEST",      name), LogicOp::BitwiseOr,  self.ident,    test_wire_i,
            None, None,
        );
        // (self | testWire) == upFullState
        let end_expr = model_ar.make_expression(
            false, &format!("{}_END_EXPR",     name), LogicOp::RelationEq, or_expr,       up_full_state_i,
            None, None,
        );
        self.end_expr_i = Some(end_expr);

        // ~endExpr  (unary — b operand is ignored)
        let end_expr_inv = model_ar.make_expression(
            false, &format!("{}_END_EXPR_INV", name), LogicOp::BitwiseInvr, end_expr, end_expr,
            None, None,
        );
        self.end_expr_inv_i = Some(end_expr_inv);
    }

    /// Registers all update events in the arena.
    /// `build_support_signal` must be called first.
    pub fn build_update_event(&mut self, model_ar: &mut ModelArena) {

        let owner_name = self.build_unique_name().to_string();
        self.triggers.integrity_check(&owner_name, model_ar);

        let up_state_i        = self.up_state_i       .expect("build_support_signal must be called first");
        let down_full_state_i = self.down_full_state_i.expect("build_support_signal must be called first");
        let test_wire_i       = self.test_wire_i      .expect("build_support_signal must be called first");
        let end_expr_i        = self.end_expr_i       .expect("build_support_signal must be called first");
        let end_expr_inv_i    = self.end_expr_inv_i   .expect("build_support_signal must be called first");

        let full_sl = Slice::new(0, self.bit_width);
        let bit_sl  = Slice::new(0, 1);
        let cm      = self.retrieve_clk_mode();

        // create the update event for the unset signal
        let ue_unset = model_ar.make_ue_full(
            None, Some(end_expr_i), down_full_state_i,
            full_sl, full_sl,
            DEFAULT_UE_PRI_SY_UNSET, cm, false,
        );
        self.add_update_event(ue_unset);

        // create the update event for the set signal
        let mut fill_bit_idx: i32 = 0;
        let nodes: Vec<_> = self.triggers.iter_depend_nodes().collect();
        for (srci, condi) in nodes {
            let des_sl = Slice::new(fill_bit_idx, fill_bit_idx + 1);

            // actual condition = should we update this bit into register?
            let actual_cond = 
                match condi {
                    Some(ci) => {
                        let combined = model_ar.make_expression(
                            false,
                            &format!("{}_ACT_COND_{}", self.ident.get_ident_base().get_name(), fill_bit_idx),
                            LogicOp::BitwiseAnd, end_expr_inv_i, ci,
                            None, None,
                        );
                        Some(combined)
                    }
                    None => Some(end_expr_inv_i),
                };

            // main UE on self: guarded by endExprInv so we never re-fill an already-full reg
            let ue = model_ar.make_ue_full(
                actual_cond, Some(srci), up_state_i,
                des_sl     , bit_sl     ,
                DEFAULT_UE_PRI_SY_ACTIVATE, cm, false,
            );
            self.add_update_event(ue);

            // test UE on testWire: same activation but CLK_FREE, no endExpr gate
            let test_ue = model_ar.make_ue_full(
                condi, Some(srci), up_state_i,
                des_sl, bit_sl,
                DEFAULT_UE_PRI_SY_ACTIVATE, ClockMode::ClkFree, false,
            );
            let mut tw = model_ar.take_hcp(test_wire_i);
            tw.add_update_event(test_ue);
            model_ar.replace_back_hcp(tw);

            fill_bit_idx += 1;
        }

        assert_eq!(
            fill_bit_idx, self.bit_width,
            "SyncReg '{}': {} of {} bit slots filled",
            self.ident.get_ident_base().get_name(), fill_bit_idx, self.bit_width
        );

        // create the update event for the hold signal
        if let Some(hold_sig_i) = self.get_hold_sig_i() {
            let ue = model_ar.make_ue_full(
                None, Some(hold_sig_i), self.ident,
                full_sl, full_sl,
                DEFAULT_UE_PRI_SY_HOLD, self.retrieve_clk_mode(), false
            );
            self.add_update_event(ue);
        }

        // create the update event for the reset signal
        if let Some(rst_sig_i) = self.get_rst_sig_i() {
            let ue = model_ar.make_ue_full(
                None                 , Some(rst_sig_i)         , down_full_state_i,
                full_sl              , full_sl                 ,
                DEFAULT_UE_PRI_SY_RST, self.retrieve_clk_mode(), false
            );
            self.add_update_event(ue);
        }

        // create the update event for the MASTER reset signal
        if let Some(mrst_sig_i) = self.get_mrst_sig_i() {
            let ue = model_ar.make_ue_full(
                None                  , Some(mrst_sig_i)        , down_full_state_i,
                full_sl               , full_sl                 ,
                DEFAULT_UE_PRI_SY_MRST, self.retrieve_clk_mode(), false
            );
            self.add_update_event(ue);
        }
    }
}

impl HasTriggerSig for SyncReg {
    fn get_triggers    (&self)     -> &TriggerSig     { &self.triggers     }
    fn get_triggers_mut(&mut self) -> &mut TriggerSig { &mut self.triggers }
}

impl HcpAssignable for SyncReg {
    fn get_hcp_assign    (&self)     -> &    HcpAssign { &self.assign }
    fn get_hcp_assign_mut(&mut self) -> &mut HcpAssign { &mut self.assign }

    fn retrieve_clk_mode(&self) -> ClockMode { ClockMode::PosEdge }
    fn get_des_slice    (&self) -> Slice     { Slice::new(0, self.bit_width) }
    fn get_priority     (&self) -> i32       { DEFAULT_UE_PRI_INTERNAL_MIN }

    fn do_asm(&self,
              _srci: HcpIdent,
              _des_slice: Option<Slice>,
              _src_slice: Slice,
              _arena    : &mut ModelArena) -> AssignMeta {
        panic!("SyncReg::do_asm() is not supported; use build_update_event()")
    }
}

impl Identifiable for SyncReg {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}

impl HcpIdentifiable for SyncReg {
    fn get_ident_mut(&mut self) -> &mut HcpIdent { &mut self.ident }
}

impl HcpBase for SyncReg {
    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_sync_reg(*self); }
}
