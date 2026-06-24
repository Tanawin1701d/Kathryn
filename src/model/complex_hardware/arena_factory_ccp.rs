use crate::model::complex_hardware::arb::{Arb, ArbSamePriPolicy};
use crate::model::complex_hardware::common::ccp_ident::CcpIdent;
use crate::model::complex_hardware::karray::Karray;
use crate::model::hw_component::common::hcp_ident::HwComponentType;
use crate::model::model_arena::ModelArena;

// make_* → is_user_com = false (internal/system)
// mk_*   → is_user_com = true  (user-defined)
//
// CCPs are stamped into the module on the trace stack (like HCPs), but are NOT
// part of the flow graph (unlike NCPs). The owning module builds them during
// build_flow_base.

impl ModelArena {
    // ---- Arb ---------------------------------------------------------------
    pub fn make_arb(&mut self, is_user_com: bool, name: &str, policy: ArbSamePriPolicy) -> CcpIdent {
        let a = Arb::new(is_user_com, name, policy, self);
        let i = self.add_arb(a);
        self.stamp_ccp_to_parent_module(i)
    }

    // ---- Karray ------------------------------------------------------------
    pub fn make_karray(
        &mut self,
        is_user_com: bool,
        name       : &str,
        shape      : Vec<usize>,
        fields     : Vec<(String, i32)>,
        backing    : HwComponentType,
    ) -> CcpIdent {
        let k = Karray::new(is_user_com, name, shape, fields, backing, self);
        let i = self.add_karray(k);
        self.stamp_ccp_to_parent_module(i)
    }
}
