use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::hcp_base::HcpBase;
use crate::model::hw_component::common::hcp_assign::{HcpAssign, HcpAssignable};
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HcpIdentifiable, HwComponentType};
use crate::model::hw_component::sp_reg::trigger_sig::{HasTriggerSig, TriggerSig};
use crate::model::hw_component::common::slice::Slice;
use crate::model::hw_component::common::update_event::{DEFAULT_UE_PRI_INTERNAL_MIN, DEFAULT_UE_PRI_RST};

// ---- UE priority ladder: lower wins on conflict; MRST sits at the global RST band ----
const DEFAULT_UE_PRI_SR_UNSET : i32 = DEFAULT_UE_PRI_INTERNAL_MIN;     // default-clear, always loses
const DEFAULT_UE_PRI_SR_SET   : i32 = DEFAULT_UE_PRI_INTERNAL_MIN + 1; // user trigger sets the bit
const DEFAULT_UE_PRI_SR_HOLD  : i32 = DEFAULT_UE_PRI_INTERNAL_MIN + 2; // hold overrides set
const DEFAULT_UE_PRI_SR_RST   : i32 = DEFAULT_UE_PRI_INTERNAL_MIN + 3; // soft reset overrides hold
const DEFAULT_UE_PRI_SR_INT   : i32 = DEFAULT_UE_PRI_INTERNAL_MIN + 4; // interrupt overrides reset
const DEFAULT_UE_PRI_SR_MRST  : i32 = DEFAULT_UE_PRI_RST;              // master reset wins over all
use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::model_arena::ModelArena;
use crate::model::nodes::ncp_ident::NcpIdent;

/// 1-bit state register.  Mirrors C++ `StateReg`.

#[derive(Default)]
pub struct StateReg {
    // hcp ident
    assign      : HcpAssign,
    ident       : HcpIdent,
    // trigger signals
    triggers    : TriggerSig,
    // set/unset signals
    set_val_i   : HcpIdent,
    unset_val_i : HcpIdent,
    // bitwidth is set to 1
}

impl StateReg {
    // ---- constructors ----

    /// Full constructor; spawns the `_SET`/`_UNSET` constant Vals in the arena so
    /// callers don't have to pre-build them.
    pub fn new(is_user_com: bool,
               name       : &str,
               arena      : &mut ModelArena) -> Self {
        let set_val_i   = arena.make_val(false, &format!("{}_SET",   name), 1, 1);
        let unset_val_i = arena.make_val(false, &format!("{}_UNSET", name), 1, 0);
        Self {
            assign      : HcpAssign::new(),
            ident       : HcpIdent::new(HwComponentType::StateReg, is_user_com, name),
            triggers    : TriggerSig::new(),
            set_val_i,
            unset_val_i
        }
    }

    // ---- accessors ----

    pub fn get_ident    (&    self) ->      HcpIdent { self.ident }
    pub fn get_ident_mut(&mut self) -> &mut HcpIdent { &mut self.ident }

    // ---- update-event build phase ----

    /// Emits one UE per trigger source (unset/set/hold/rst/int/mrst); `clk_src` is
    /// derived once from `triggers.clk_sig_i` and passed to every UE so they all
    /// share the same clock domain and are joinable in the update pool.
    pub fn build_update_event(&mut self, model_ar: &mut ModelArena) {

        let owner_name = self.build_unique_name().to_string();
        self.triggers.integrity_check(&owner_name, model_ar);

        // in source side, we also use get_des_slice because it is only 1 bit assignment
        let src_sl = Slice::new(0, 1);

        let clk_src = self.get_clk_sig_i();

        // create the update event for the unset signal
        let ue = model_ar.make_ue_full(
            None                   , None                    , self.unset_val_i,
            self.get_des_slice()   , src_sl                  ,
            DEFAULT_UE_PRI_SR_UNSET, self.retrieve_clk_mode(), false, clk_src
        );
        self.add_update_event(ue);

        // create the update event for the set signal
        let nodes: Vec<_> = self.triggers.iter_depend_nodes().collect();
        for (srci, condi) in nodes {
            let des_slice = self.get_des_slice();
            let src_slice = src_sl;
            let priority = DEFAULT_UE_PRI_SR_SET;
            let cm = self.retrieve_clk_mode();
            let ue = model_ar.make_ue_full(
                condi    , Some(srci), self.set_val_i,
                des_slice, src_slice ,
                priority , cm        , false, clk_src
            );
            self.add_update_event(ue);
        }

        // create the update event for the hold signal
        if let Some(hold_sig_i) = self.get_hold_sig_i() {
            let ue = model_ar.make_ue_full(
                None                  , Some(hold_sig_i)        , self.set_val_i,
                self.get_des_slice()  , src_sl                  ,
                DEFAULT_UE_PRI_SR_HOLD, self.retrieve_clk_mode(), false, clk_src
            );
            self.add_update_event(ue);
        }

        // create the update event for the reset signal
        if let Some(rst_sig_i) = self.get_rst_sig_i() {
            let ue = model_ar.make_ue_full(
                None                 , Some(rst_sig_i)          , self.unset_val_i,
                self.get_des_slice() , src_sl                   ,
                DEFAULT_UE_PRI_SR_RST, self.retrieve_clk_mode() , false, clk_src
            );
            self.add_update_event(ue);
        }

        // create the update event for the interrupt signal
        if let Some(int_sig_i) = self.get_int_sig_i() {
            let ue = model_ar.make_ue_full(
                None                 , Some(int_sig_i)          , self.set_val_i,
                self.get_des_slice() , src_sl                   ,
                DEFAULT_UE_PRI_SR_INT, self.retrieve_clk_mode() , false, clk_src
            );
            self.add_update_event(ue);
        }

        // create the update event for the MASTER reset signal
        if let Some(mrst_sig_i) = self.get_mrst_sig_i() {
            let ue = model_ar.make_ue_full(
                None                  , Some(mrst_sig_i)         , self.unset_val_i,
                self.get_des_slice()  , src_sl                   ,
                DEFAULT_UE_PRI_SR_MRST, self.retrieve_clk_mode() , false, clk_src
            );
            self.add_update_event(ue);
        }
    }
}

impl HasTriggerSig for StateReg {
    fn get_triggers    (&self)     -> &TriggerSig     { &self.triggers     }
    fn get_triggers_mut(&mut self) -> &mut TriggerSig { &mut self.triggers }
}

// StateReg is always 1-bit, posedge-clocked, and never assigned via do_asm —
// all writes flow through build_update_event's pre-built UEs.
impl HcpAssignable for StateReg {
    fn get_hcp_assign    (&self)     -> &    HcpAssign { &self.assign }
    fn get_hcp_assign_mut(&mut self) -> &mut HcpAssign { &mut self.assign }

    fn retrieve_clk_mode(&self) -> ClockMode { ClockMode::PosEdge }
    fn get_des_slice    (&self) -> Slice     { Slice::new(0, 1) }
    fn get_priority     (&self) -> i32       { DEFAULT_UE_PRI_SR_UNSET }

    fn do_asm(&self,
              _srci     : HcpIdent,
              _des_slice: Option<Slice>,
              _src_slice: Slice,
              _arena    : &mut ModelArena) -> NcpIdent {
        panic!("StateReg::do_asm() is not supported; use build_update_event()")
    }
}

impl Identifiable for StateReg {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}

impl HcpIdentifiable for StateReg {
    fn get_ident_mut(&mut self) -> &mut HcpIdent { &mut self.ident }
}

impl HcpBase for StateReg {
    // Each concrete type knows its own arena slot, so callers use zero match.
    fn replace_back_into_arena(self: Box<Self>, arena: &mut ModelArena) { arena.replace_back_state_reg(*self); }
}
