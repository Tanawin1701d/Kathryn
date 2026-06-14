use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::hcp_base::HcpBase;
use crate::model::hw_component::common::hcp_assign::{HcpAssign, HcpAssignable};
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HcpIdentifiable, HcpSensitiveType, HwComponentType};
use crate::model::hw_component::common::operation::LogicOp;
use crate::model::hw_component::common::slice::Slice;
use crate::model::hw_component::common::update_event::{DEFAULT_UE_PRI_INTERNAL_MIN, DEFAULT_UE_PRI_RST};
use crate::model::hw_component::sp_reg::trigger_sig::{HasTriggerSig, TriggerSig};
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::NcpIdent;
use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::util::math::vary_val::VaryVal;

// ---- UE priority ladder: higher value wins; MRST sits at the global RST band ----
const DEFAULT_UE_PRI_SY_UNSET    : i32 = DEFAULT_UE_PRI_INTERNAL_MIN;     // clear-on-full, lowest
const DEFAULT_UE_PRI_SY_ACTIVATE : i32 = DEFAULT_UE_PRI_INTERNAL_MIN + 1; // per-bit trigger fill
const DEFAULT_UE_PRI_SY_HOLD     : i32 = DEFAULT_UE_PRI_INTERNAL_MIN + 2; // hold overrides activate
const DEFAULT_UE_PRI_SY_RST      : i32 = DEFAULT_UE_PRI_INTERNAL_MIN + 3; // soft reset overrides hold
// INTERRUPT is not supported, so we use the same priority as RST
const DEFAULT_UE_PRI_SY_MRST     : i32 = DEFAULT_UE_PRI_RST;              // master reset wins over all

/// n-bit synchronisation register: each bit latches independently when its trigger fires,
/// and the whole register clears in one cycle once every bit has been raised.
pub struct SyncReg {
    assign            : HcpAssign,
    ident             : HcpIdent,
    bit_width         : i32,                 // number of independent bit slots
    triggers          : TriggerSig,
    // support signals (populated by build_support_signal)
    up_state_i        : Option<HcpIdent>,    // 1-bit Val = 1
    up_full_state_i   : Option<HcpIdent>,    // n-bit Val = all 1s
    down_full_state_i : Option<HcpIdent>,    // n-bit Val = all 0s
    test_wire_i       : Option<HcpIdent>,    // n-bit Wire (observes activations without endExpr gate)
    end_expr_i        : Option<HcpIdent>,    // (self | testWire) == upFullState
    end_expr_inv_i    : Option<HcpIdent>,    // ~endExpr — gate that suppresses re-fills
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
    // ---- constructors ----

    /// Full constructor; asserts `size > 0` so the bit-slot indexing in
    /// build_update_event cannot overflow.
    pub fn new(is_user_com: bool, name: &str, size: i32) -> Self {
        assert!(size > 0, "SyncReg size must be positive");
        Self {
            assign            : HcpAssign::new(),
            ident             : HcpIdent::new(HwComponentType::SyncReg, HcpSensitiveType::Clocked, is_user_com, name),
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

    // ---- accessors ----

    pub fn get_ident       (&    self) -> HcpIdent         { self.ident      }
    pub fn get_ident_mut   (&mut self) -> &mut HcpIdent    { &mut self.ident }
    pub fn get_end_expr_i  (&    self) -> Option<HcpIdent> { self.end_expr_i }

    // ---- build phase: support signals + update events ----

    /// Creates all internal support signals in the arena.
    /// Must be called before `build_update_event`.
    pub fn build_support_signal(&mut self, model_ar: &mut ModelArena) {
        let name = self.ident.get_ident_base().get_abs_name().to_string();
        let size = self.bit_width;

        let up_state_i = model_ar.make_val(false, &format!("{}_UP_STATE", name), 1, 1);
        self.up_state_i = Some(up_state_i);

        let up_full_state_i = model_ar.make_val_vv(
            false, &format!("{}_UP_FULL", name), size,
            VaryVal::all_ones(size as usize),
        );
        self.up_full_state_i = Some(up_full_state_i);

        let down_full_state_i = model_ar.make_val(false, &format!("{}_DOWN_FULL", name), size, 0);
        self.down_full_state_i = Some(down_full_state_i);

        // test_wire mirrors per-bit activations without the endExpr gate, so end-detection
        // can see "this cycle would fill the last bit" before the gate suppresses the write.
        let test_wire_i = model_ar.make_wire(false, &format!("{}_TEST_WIRE", name), size);
        self.test_wire_i = Some(test_wire_i);

        let self_sl = Slice::new(0, size);

        // (self | testWire)
        let or_expr = model_ar.make_expression(
            false, &format!("{}_OR_TEST",      name), LogicOp::BitwiseOr,  self.ident,    test_wire_i,
            Some(self_sl), Some(self_sl),
        );
        // (self | testWire) == upFullState
        let end_expr = model_ar.make_expression(
            false, &format!("{}_END_EXPR",     name), LogicOp::RelationEq, or_expr,       up_full_state_i,
            None, None,
        );
        self.end_expr_i = Some(end_expr);

        // ~endExpr  (unary)
        let end_expr_inv = model_ar.make_expression_single(
            false, &format!("{}_END_EXPR_INV", name), LogicOp::BitwiseInvr, end_expr,
            None,
        );
        self.end_expr_inv_i = Some(end_expr_inv);
    }

    /// Registers all update events in the arena. Each trigger gets its own bit slot
    /// (assigned in dependency-iteration order) and a paired CLK_FREE test-wire UE so
    /// the endExpr can see the final fill in the same cycle.  `clk_src` is derived
    /// once from triggers.clk_sig_i and shared across every clocked UE for joinability.
    /// `build_support_signal` must be called first.
    pub fn build_update_event(&mut self, model_ar: &mut ModelArena) {

        let owner_name = self.get_global_name().to_string();
        self.triggers.integrity_check(&owner_name, model_ar, Some(self.ident), self.get_des_slice().get_size());

        let up_state_i        = self.up_state_i       .expect("build_support_signal must be called first");
        let down_full_state_i = self.down_full_state_i.expect("build_support_signal must be called first");
        let test_wire_i       = self.test_wire_i      .expect("build_support_signal must be called first");
        let end_expr_i        = self.end_expr_i       .expect("build_support_signal must be called first");
        let end_expr_inv_i    = self.end_expr_inv_i   .expect("build_support_signal must be called first");

        let full_sl = Slice::new(0, self.bit_width);
        let bit_sl  = Slice::new(0, 1);
        let cm      = self.retrieve_clk_mode();
        let clk_src = self.get_clk_sig_i();

        // create the update event for the unset signal
        let ue_unset = model_ar.make_ue_full(
            None, Some(end_expr_i), down_full_state_i,
            full_sl, full_sl,
            DEFAULT_UE_PRI_SY_UNSET, cm, false, clk_src,
        );
        self.add_update_event(ue_unset);

        // create the update event for the set signal
        // fill_bit_idx walks left→right across the bit_width, assigning one slot per
        // trigger; the assert below enforces #triggers == bit_width so every slot is
        // covered exactly once.
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
                            &format!("{}_ACT_COND_{}", self.ident.get_ident_base().get_abs_name(), fill_bit_idx),
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
                DEFAULT_UE_PRI_SY_ACTIVATE, cm, false, clk_src,
            );
            self.add_update_event(ue);

            // test UE on testWire: same activation but CLK_FREE, no endExpr gate —
            // the wire reflects the activation combinationally so endExpr can detect
            // the last fill in the same cycle the main UE is firing.
            let test_ue = model_ar.make_ue_full(
                condi, Some(srci), up_state_i,
                des_sl, bit_sl,
                DEFAULT_UE_PRI_SY_ACTIVATE, ClockMode::ClkFree, false, None,
            );
            // take/replace_back leaves `model_ar` free for the test_ue creation above
            // and any future arena access inside add_update_event.
            let mut tw = model_ar.take_hcp(test_wire_i);
            tw.add_update_event(test_ue);
            model_ar.replace_back_hcp(tw);

            fill_bit_idx += 1;
        }

        assert_eq!(
            fill_bit_idx, self.bit_width,
            "SyncReg '{}': {} of {} bit slots filled",
            self.ident.get_ident_base().get_abs_name(), fill_bit_idx, self.bit_width
        );

        // create the update event for the hold signal
        if let Some(hold_sig_i) = self.get_hold_sig_i() {
            let ue = model_ar.make_ue_full(
                None, Some(hold_sig_i), self.ident,
                full_sl, full_sl,
                DEFAULT_UE_PRI_SY_HOLD, self.retrieve_clk_mode(), false, clk_src
            );
            self.add_update_event(ue);
        }

        // create the update event for the reset signal
        if let Some(rst_sig_i) = self.get_rst_sig_i() {
            let ue = model_ar.make_ue_full(
                None                 , Some(rst_sig_i)         , down_full_state_i,
                full_sl              , full_sl                 ,
                DEFAULT_UE_PRI_SY_RST, self.retrieve_clk_mode(), false, clk_src
            );
            self.add_update_event(ue);
        }

        // create the update event for the MASTER reset signal
        if let Some(mrst_sig_i) = self.get_mrst_sig_i() {
            let ue = model_ar.make_ue_full(
                None                  , Some(mrst_sig_i)        , down_full_state_i,
                full_sl               , full_sl                 ,
                DEFAULT_UE_PRI_SY_MRST, self.retrieve_clk_mode(), false, clk_src
            );
            self.add_update_event(ue);
        }
    }
}

impl HasTriggerSig for SyncReg {
    fn get_triggers    (&self)     -> &TriggerSig     { &self.triggers     }
    fn get_triggers_mut(&mut self) -> &mut TriggerSig { &mut self.triggers }
}

// SyncReg is always posedge-clocked and never assigned via do_asm —
// all writes flow through build_update_event's per-bit UEs.
impl HcpAssignable for SyncReg {
    fn get_hcp_assign    (&self)     -> &    HcpAssign { &self.assign }
    fn get_hcp_assign_mut(&mut self) -> &mut HcpAssign { &mut self.assign }

    fn retrieve_clk_mode(&self) -> ClockMode { ClockMode::PosEdge }
    fn get_des_slice    (&self) -> Slice     { Slice::new(0, self.bit_width) }
    fn get_priority     (&self) -> i32       { DEFAULT_UE_PRI_INTERNAL_MIN }

    fn do_asm(&self,
              _srci     : HcpIdent,
              _des_slice: Option<Slice>,
              _src_slice: Slice,
              _arena    : &mut ModelArena) -> NcpIdent {
        panic!("SyncReg::do_asm() is not supported; use build_update_event()")
    }
}

impl Identifiable for SyncReg {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
}

impl HcpIdentifiable for SyncReg {
    fn get_ident    (&    self) ->      HcpIdent { self.ident      }
    fn get_ident_mut(&mut self) -> &mut HcpIdent { &mut self.ident }
}

impl HcpBase for SyncReg {
    // Each concrete type knows its own arena slot, so callers use zero match.
    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_sync_reg(*self); }
}
