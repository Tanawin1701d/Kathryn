use crate::model::common::identifier::Identifiable;
use crate::model::hw_component::common::update_event::{UeBasic, UeCond, UeCommon, UeGrp, UeSwitch, UeType};
use crate::model::hw_component::common::update_event_ident::UpdateEventIdent;
use crate::model::model_arena::ModelArena;

macro_rules! dispatch_ue_common {
    ($self:expr, $ident:expr) => {{
        let h = *$ident.get_arena_handle();
        match $ident.get_ue_type() {
            UeType::Basic  => $self.ue_basics  .get(h).ue_common(),
            UeType::Grp    => $self.ue_grps    .get(h).ue_common(),
            UeType::Cond   => $self.ue_conds   .get(h).ue_common(),
            UeType::Switch => $self.ue_switches.get(h).ue_common(),
            UeType::Untype => panic!("UpdateEventIdent has UeType::Untype"),
        }
    }};
}

impl ModelArena {
    // -----------------------------------------------------------------------
    // Update-event typed inserts — stamp arena_handle, return UpdateEventIdent
    // -----------------------------------------------------------------------
    pub fn insert_ue_basic(&mut self, e: UeBasic) -> UpdateEventIdent {
        let h = self.ue_basics.insert(e);
        self.ue_basics.get(h).ident()
    }

    pub fn insert_ue_grp(&mut self, e: UeGrp) -> UpdateEventIdent {
        let h = self.ue_grps.insert(e);
        self.ue_grps.get(h).ident()
    }

    pub fn insert_ue_cond(&mut self, e: UeCond) -> UpdateEventIdent {
        let h = self.ue_conds.insert(e);
        self.ue_conds.get(h).ident()
    }

    pub fn insert_ue_switch(&mut self, e: UeSwitch) -> UpdateEventIdent {
        let h = self.ue_switches.insert(e);
        self.ue_switches.get(h).ident()
    }

    // -----------------------------------------------------------------------
    // UE take / replace_back — owned-access path (mirrors HCP pattern)
    // -----------------------------------------------------------------------
    pub fn take_ue_basic (&mut self, h: UpdateEventIdent) -> UeBasic  { self.ue_basics  .take(*h.get_arena_handle()) }
    pub fn take_ue_grp   (&mut self, h: UpdateEventIdent) -> UeGrp    { self.ue_grps    .take(*h.get_arena_handle()) }
    pub fn take_ue_cond  (&mut self, h: UpdateEventIdent) -> UeCond   { self.ue_conds   .take(*h.get_arena_handle()) }
    pub fn take_ue_switch(&mut self, h: UpdateEventIdent) -> UeSwitch { self.ue_switches.take(*h.get_arena_handle()) }

    pub fn replace_back_ue_basic (&mut self, v: UeBasic)  { let h = *v.get_arena_handle(); self.ue_basics  .replace_back(h, v) }
    pub fn replace_back_ue_grp   (&mut self, v: UeGrp)    { let h = *v.get_arena_handle(); self.ue_grps    .replace_back(h, v) }
    pub fn replace_back_ue_cond  (&mut self, v: UeCond)   { let h = *v.get_arena_handle(); self.ue_conds   .replace_back(h, v) }
    pub fn replace_back_ue_switch(&mut self, v: UeSwitch) { let h = *v.get_arena_handle(); self.ue_switches.replace_back(h, v) }

    /// Resolve an `UpdateEventIdent` to its `UeCommon` (priority, clk_mode, etc.)
    /// Trait-/shared-view accessor; cannot be expressed via take/replace.
    pub fn get_ue_common(&self, ident: &UpdateEventIdent) -> &UeCommon {
        dispatch_ue_common!(self, ident)
    }
}
