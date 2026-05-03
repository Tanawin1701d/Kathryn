use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::hcp_accesible::HcpAccessible;
use crate::model::hw_component::common::hcp_assign::{HcpAssign, HcpAssignable};
use crate::model::hw_component::common::hcp_ident::{HcpIdent, HwComponentType};
use crate::model::hw_component::sp_reg::trigger_sig::{HasTriggerSig, TriggerSig};
use crate::model::hw_component::common::hcp_read::HcpReadable;
use crate::model::hw_component::common::slice::Slice;
use crate::model::hw_component::common::update_event::{DEFAULT_UE_PRI_INTERNAL_MIN, DEFAULT_UE_PRI_RST};

const DEFAULT_UE_PRI_SR_UNSET : i32 = DEFAULT_UE_PRI_INTERNAL_MIN;
const DEFAULT_UE_PRI_SR_SET   : i32 = DEFAULT_UE_PRI_INTERNAL_MIN + 1;
const DEFAULT_UE_PRI_SR_HOLD  : i32 = DEFAULT_UE_PRI_INTERNAL_MIN + 2;
const DEFAULT_UE_PRI_SR_RST   : i32 = DEFAULT_UE_PRI_INTERNAL_MIN + 3;
const DEFAULT_UE_PRI_SR_INT   : i32 = DEFAULT_UE_PRI_INTERNAL_MIN + 4;
const DEFAULT_UE_PRI_SR_MRST  : i32 = DEFAULT_UE_PRI_RST;
use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::model_arena::ModelArena;

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
    pub fn new(is_user_com: bool,
               name       : &str,
               set_val_i  : HcpIdent,
               unset_val_i: HcpIdent) -> Self {
        Self {
            assign      : HcpAssign::new(),
            ident       : HcpIdent::new(HwComponentType::StateReg, is_user_com, name),
            triggers    : TriggerSig::new(),
            set_val_i,
            unset_val_i
        }
    }

    pub fn mk(name       : &str,
              set_val_i  : HcpIdent,
              unset_val_i: HcpIdent) -> Self {
        Self::new(false, name, set_val_i, unset_val_i)
    }

    pub fn get_ident(&self) -> HcpIdent { self.ident }

    pub fn build_update_event(&mut self, model_ar: &mut ModelArena) {

        let owner_name = self.build_unique_name().to_string();
        self.triggers.integrity_check(&owner_name, model_ar);

        // in source side, we also use get_des_slice because it is only 1 bit assignment
        let src_sl = Slice::new(0, 1);

        // create the update event for the unset signal
        let ue = model_ar.make_ue_full(
            None                   , None                    , self.unset_val_i,
            self.get_des_slice()   , src_sl                  ,
            DEFAULT_UE_PRI_SR_UNSET, self.retrieve_clk_mode(), false
        );
        self.add_update_event(ue);

        // create the update event for the set signal
        let nodes = self.triggers;
        for (srci, condi) in nodes.iter_depend_nodes() {
            let des_slice = self.get_des_slice();
            let src_slice = src_sl;
            let priority = DEFAULT_UE_PRI_SR_SET;
            let cm = self.retrieve_clk_mode();
            let ue = model_ar.make_ue_full(
                condi    , Some(srci), self.set_val_i,
                des_slice, src_slice ,
                priority , cm        , false
            );
            self.add_update_event(ue);
        }

        // create the update event for the hold signal
        if let Some(hold_sig_i) = self.get_hold_sig_i() {
            let ue = model_ar.make_ue_full(
                None                  , Some(hold_sig_i)        , self.set_val_i,
                self.get_des_slice()  , src_sl                  ,
                DEFAULT_UE_PRI_SR_HOLD, self.retrieve_clk_mode(), false
            );
            self.add_update_event(ue);
        }

        // create the update event for the reset signal
        if let Some(rst_sig_i) = self.get_rst_sig_i() {
            let ue = model_ar.make_ue_full(
                None                 , Some(rst_sig_i)          , self.unset_val_i,
                self.get_des_slice() , src_sl                   ,
                DEFAULT_UE_PRI_SR_RST, self.retrieve_clk_mode() , false
            );
            self.add_update_event(ue);
        }

        // create the update event for the interrupt signal
        if let Some(int_sig_i) = self.get_int_sig_i() {
            let ue = model_ar.make_ue_full(
                None                 , Some(int_sig_i)          , self.set_val_i,
                self.get_des_slice() , src_sl                   ,
                DEFAULT_UE_PRI_SR_INT, self.retrieve_clk_mode() , false
            );
            self.add_update_event(ue);
        }

        // create the update event for the MASTER reset signal
        if let Some(mrst_sig_i) = self.get_mrst_sig_i() {
            let ue = model_ar.make_ue_full(
                None                  , Some(mrst_sig_i)         , self.unset_val_i,
                self.get_des_slice()  , src_sl                   ,
                DEFAULT_UE_PRI_SR_MRST, self.retrieve_clk_mode() , false
            );
            self.add_update_event(ue);
        }
    }
}

impl HasTriggerSig for StateReg {
    fn get_triggers    (&self)     -> &TriggerSig     { &self.triggers     }
    fn get_triggers_mut(&mut self) -> &mut TriggerSig { &mut self.triggers }
}

impl HcpReadable for StateReg {
    fn get_hcp_rdb_ident(&self) -> HcpIdent { self.ident }
}

impl HcpAssignable for StateReg {
    fn get_hcp_assign    (&self)     -> &    HcpAssign { &self.assign }
    fn get_hcp_assign_mut(&mut self) -> &mut HcpAssign { &mut self.assign }

    fn get_hcp_asb_ident(&self) -> HcpIdent { self.ident }
    fn retrieve_clk_mode(&self) -> ClockMode { ClockMode::PosEdge }
    fn get_des_slice    (&self) -> Slice     { Slice::new(0, 1) }
    fn get_priority     (&self) -> i32       { DEFAULT_UE_PRI_SR_UNSET }

    fn do_asm(&self,
              _srci     : &HcpIdent,
              _des_slice: &Option<Slice>,
              _src_slice: &Slice,
              _arena    : &mut ModelArena) -> AssignMeta {
        panic!("StateReg::do_asm() is not supported; use build_update_event()")
    }
}

impl HcpAccessible for StateReg {
    fn get_bit_width(&self) -> usize { 1 }
}

impl Identifiable for StateReg {
    fn get_ident_base    (&self)     -> &IdentBase     { self.ident.get_ident_base()     }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { self.ident.get_ident_base_mut() }
    fn build_unique_name (&mut self) -> &str           { self.ident.build_unique_name()  }
}
