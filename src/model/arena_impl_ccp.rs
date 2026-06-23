use crate::model::common::identifier::Identifiable;
use crate::model::complex_hardware::arb::Arb;
use crate::model::complex_hardware::common::ccp_base::CcpBase;
use crate::model::complex_hardware::common::ccp_ident::{CcpIdent, CcpType};
use crate::model::complex_hardware::karray::Karray;
use crate::model::model_arena::{ModelArena, ModuleInitStage};

// CRUD + module stamping + build dispatch for complex component properties
// (CCPs).  Only one CCP type exists today (Arb); the type-keyed helpers carry a
// single match arm so a new CCP type is a one-line addition.

impl ModelArena {
    // ----- inserts ---------------------------------------------------------
    pub fn add_arb   (&mut self, a: Arb)    -> CcpIdent { let h = self.arbs   .insert(a); self.arbs   .get(h).get_ccp_ident() }
    pub fn add_karray(&mut self, k: Karray) -> CcpIdent { let h = self.karrays.insert(k); self.karrays.get(h).get_ccp_ident() }

    // ----- take / replace_back (use these instead of typed get/get_mut) ----
    pub fn take_arb           (&mut self, i: CcpIdent) -> Arb    { self.arbs   .take(*i.get_arena_handle()) }
    pub fn replace_back_arb   (&mut self, v: Arb)                { let h = *v.get_arena_handle(); self.arbs   .replace_back(h, v) }
    pub fn take_karray        (&mut self, i: CcpIdent) -> Karray { self.karrays.take(*i.get_arena_handle()) }
    pub fn replace_back_karray(&mut self, v: Karray)             { let h = *v.get_arena_handle(); self.karrays.replace_back(h, v) }

    // ----- polymorphic take / replace_back (CcpBase dispatch) --------------
    // ONE match for CCP → CcpBase. Add one arm per new CCP type; nothing else in
    // the CCP plumbing changes. replace_back needs no match (each type knows its slot).
    pub fn take_ccp(&mut self, i: CcpIdent) -> Box<dyn CcpBase> {
        match i.get_ccp_type() {
            CcpType::Arb    => Box::new(self.take_arb(i)),
            CcpType::Karray => Box::new(self.take_karray(i)),
        }
    }
    pub fn replace_back_ccp(&mut self, ccp: Box<dyn CcpBase>) { ccp.replace_back_into_arena(self); }

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

    // Write the (re-stamped) ident back into the stored CCP object (zero match).
    fn write_back_ccp_ident(&mut self, i: CcpIdent) {
        let mut ccp = self.take_ccp(i);
        ccp.set_ccp_ident(i);
        self.replace_back_ccp(ccp);
    }

    // ----- build (CcpBase dispatch) ----------------------------------------

    // Build a CCP's internal hardware graph; called by the owning module during
    // build_flow_base. One arm per CCP type.
    pub fn build_ccp(&mut self, ident: CcpIdent) {
        let mut ccp = self.take_ccp(ident);
        ccp.build(self);
        self.replace_back_ccp(ccp);
    }
}
