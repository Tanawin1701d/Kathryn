use crate::model::common::identifier::Identifiable;
use crate::model::complex_hardware::common::ccp_ident::CcpIdent;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::slice::Slice;
use crate::model::model_arena::ModelArena;

// Higher-level DynCounter operations on an already-created counter CCP.
// Creation/stamping lives in arena_factory_ccp.rs; CRUD in arena_impl_ccp.rs.

impl ModelArena {
    /// Chain one add stage: `enable ? prev + addend : prev`.  Partial slices on
    /// either signal are materialised as SliceBit expressions first (the enable
    /// must resolve to exactly one bit).  Returns the new chain-head signal.
    pub fn dyn_counter_add(
        &mut self,
        ident       : CcpIdent         ,
        addend_i    : HcpIdent         ,
        addend_slice: Option<Slice>    ,
        enable_i    : Option<HcpIdent> ,
        enable_slice: Option<Slice>    ,
    ) -> Result<HcpIdent, String> {
        let base     = ident.get_ident_base().get_rel_name().to_string();
        let addend_i = self.resolve_cond_slice(&format!("{base}_addend"), addend_i, addend_slice);
        let enable_i = enable_i.map(|en_i| self.resolve_cond_slice(&format!("{base}_en"), en_i, enable_slice));
        let mut cnt  = self.take_dyn_counter(ident);
        let head     = cnt.add(addend_i, enable_i, self);
        self.replace_back_dyn_counter(cnt);
        head
    }

    /// Commit the chain head into the counter register (one clocked basic node
    /// in the current scope); the chain restarts from the register.
    pub fn dyn_counter_update(&mut self, ident: CcpIdent) -> Result<(), String> {
        let mut cnt = self.take_dyn_counter(ident);
        let res     = cnt.update(self);
        self.replace_back_dyn_counter(cnt);
        res
    }

    // ---- read accessors (take/replace_back round-trip) ---------------------

    /// The counter's backing register (the committed value).
    pub fn dyn_counter_get_reg_i(&mut self, ident: CcpIdent) -> HcpIdent {
        let cnt   = self.take_dyn_counter(ident);
        let reg_i = cnt.get_reg_i();
        self.replace_back_dyn_counter(cnt);
        reg_i
    }

    /// Head of the uncommitted add chain (the register when nothing is pending).
    pub fn dyn_counter_get_now_i(&mut self, ident: CcpIdent) -> HcpIdent {
        let cnt   = self.take_dyn_counter(ident);
        let now_i = cnt.get_now_i();
        self.replace_back_dyn_counter(cnt);
        now_i
    }

    /// Bit width of the counter register.
    pub fn dyn_counter_width(&mut self, ident: CcpIdent) -> i32 {
        let cnt   = self.take_dyn_counter(ident);
        let width = cnt.get_width();
        self.replace_back_dyn_counter(cnt);
        width
    }
}
