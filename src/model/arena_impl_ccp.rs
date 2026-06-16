use crate::model::common::identifier::Identifiable;
use crate::model::complex_hardware::arb::Arb;
use crate::model::complex_hardware::common::ccp_ident::{CcpIdent, CcpType};
use crate::model::model_arena::{ModelArena, ModuleInitStage};

// CRUD + module stamping + build dispatch for complex component properties
// (CCPs).  Only one CCP type exists today (Arb); the type-keyed helpers carry a
// single match arm so a new CCP type is a one-line addition.

impl ModelArena {
    // ----- inserts ---------------------------------------------------------
    pub fn add_arb(&mut self, a: Arb) -> CcpIdent { let h = self.arbs.insert(a); self.arbs.get(h).get_ccp_ident() }

    // ----- take / replace_back (use these instead of typed get/get_mut) ----
    pub fn take_arb        (&mut self, i: CcpIdent) -> Arb { self.arbs.take(*i.get_arena_handle()) }
    pub fn replace_back_arb(&mut self, v: Arb)            { let h = *v.get_arena_handle(); self.arbs.replace_back(h, v) }

    // ----- module stamping (mirrors stamp_hw_to_parent_module) -------------

    // Register a freshly created CCP into the module on top of the trace stack and
    // stamp master_module_i. CCPs may only be declared during the construction
    // phase (CompInit / FlowBlockInit); creating one during FlowBlockBuild is
    // disallowed (the build pass only wires existing CCPs, never makes new ones).
    pub(super) fn stamp_ccp_to_parent_module(&mut self, mut ccp_i: CcpIdent) -> CcpIdent {
        let (module_i, stage) = self.peek_module_trace_stack();
        assert!(matches!(stage, ModuleInitStage::CompInit | ModuleInitStage::FlowBlockInit),
                "stamp_ccp_to_parent_module: CCPs cannot be created during {stage:?}");
        ccp_i.set_master_module_i(module_i);
        self.write_back_ccp_ident(ccp_i);   // i must be set back into the stored CCP
        let mut m = self.take_module(module_i);
        m.add_ccp(ccp_i);
        self.replace_back_module(module_i, m);
        ccp_i
    }

    // Write the (re-stamped) ident back into the stored CCP object.
    fn write_back_ccp_ident(&mut self, i: CcpIdent) {
        match i.get_ccp_type() {
            CcpType::Arb => {
                let mut a = self.take_arb(i);
                *a.get_ccp_ident_mut() = i;
                self.replace_back_arb(a);
            }
        }
    }

    // ----- build (CcpBase dispatch) ----------------------------------------

    // Build a CCP's internal hardware graph; called by the owning module during
    // build_flow_base. One arm per CCP type.
    pub fn build_ccp(&mut self, ident: CcpIdent) {
        match ident.get_ccp_type() {
            CcpType::Arb => self.build_arb(ident),
        }
    }
}
