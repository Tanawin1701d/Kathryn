use crate::common::arena_base::ArenaHandle;
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
    // Update-event typed getters
    // -----------------------------------------------------------------------
    pub fn get_ue_basic  (&self, h: ArenaHandle) -> &UeBasic   { self.ue_basics  .get(h) }
    pub fn get_ue_grp    (&self, h: ArenaHandle) -> &UeGrp     { self.ue_grps    .get(h) }
    pub fn get_ue_cond   (&self, h: ArenaHandle) -> &UeCond    { self.ue_conds   .get(h) }
    pub fn get_ue_switch (&self, h: ArenaHandle) -> &UeSwitch  { self.ue_switches.get(h) }

    pub fn get_ue_basic_mut  (&mut self, h: ArenaHandle) -> &mut UeBasic   { self.ue_basics  .get_mut(h) }
    pub fn get_ue_grp_mut    (&mut self, h: ArenaHandle) -> &mut UeGrp     { self.ue_grps    .get_mut(h) }
    pub fn get_ue_cond_mut   (&mut self, h: ArenaHandle) -> &mut UeCond    { self.ue_conds   .get_mut(h) }
    pub fn get_ue_switch_mut (&mut self, h: ArenaHandle) -> &mut UeSwitch  { self.ue_switches.get_mut(h) }

    /// Resolve an `UpdateEventIdent` to its `UeCommon` (priority, clk_mode, etc.)
    pub fn get_ue_common(&self, ident: &UpdateEventIdent) -> &UeCommon {
        dispatch_ue_common!(self, ident)
    }
}
