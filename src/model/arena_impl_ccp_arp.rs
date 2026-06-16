use crate::model::complex_hardware::common::ccp_base::CcpBase;
use crate::model::complex_hardware::common::ccp_ident::CcpIdent;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::slice::Slice;
use crate::model::model_arena::ModelArena;

// Higher-level Arb operations (configure + build) on an already-created Arb CCP.
// Creation/stamping lives in arena_factory_ccp.rs; CRUD in arena_impl_ccp.rs.

impl ModelArena {
    /// Add a leaf (its own req/ack wires) to an existing Arb; returns its index.
    pub fn arb_add_leaf(&mut self, ident: CcpIdent, priority: i32) -> usize {
        let mut arb = self.take_arb(ident);
        let idx = arb.add_leaf(priority, self);
        self.replace_back_arb(arb);
        idx
    }

    /// Bind the single master-ack source on an existing Arb.
    pub fn arb_set_master_ack_src(&mut self, ident: CcpIdent, src_i: HcpIdent) {
        let mut arb = self.take_arb(ident);
        arb.set_master_ack_src(src_i, self);
        self.replace_back_arb(arb);
    }

    /// Bind the optional hold signal. `cond_slice` is resolved to a single bit
    /// (a partial slice is wrapped in a SliceBit expression) before the 1-bit check.
    pub fn arb_set_user_hold(&mut self, ident: CcpIdent, cond_i: HcpIdent, cond_slice: Option<Slice>) {
        let base  = ident.get_ident_base().get_rel_name().to_string();
        let sig_i = self.resolve_cond_slice(&format!("{base}_hold"), cond_i, cond_slice);
        let mut arb = self.take_arb(ident);
        arb.set_user_hold(sig_i, self);
        self.replace_back_arb(arb);
    }

    /// Bind the optional reset signal. `cond_slice` is resolved to a single bit
    /// (a partial slice is wrapped in a SliceBit expression) before the 1-bit check.
    pub fn arb_set_user_reset(&mut self, ident: CcpIdent, cond_i: HcpIdent, cond_slice: Option<Slice>) {
        let base  = ident.get_ident_base().get_rel_name().to_string();
        let sig_i = self.resolve_cond_slice(&format!("{base}_reset"), cond_i, cond_slice);
        let mut arb = self.take_arb(ident);
        arb.set_user_reset(sig_i, self);
        self.replace_back_arb(arb);
    }

    /// Wire the arbitration graph (master_req + every leaf ack). Call once after
    /// all leaves and the master-ack source are bound.
    pub fn build_arb(&mut self, ident: CcpIdent) {
        let mut arb = self.take_arb(ident);
        arb.build(self);
        self.replace_back_arb(arb);
    }
}
