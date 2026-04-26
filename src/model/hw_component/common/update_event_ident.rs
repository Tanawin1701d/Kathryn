use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::hw_component::common::update_event::{UeType, UpdatingEvent};

// ---------------------------------------------------------------------------
// UpdateEventEntry — stored in ModelArena::update_events
// ---------------------------------------------------------------------------

pub struct UpdateEventEntry {
    ident_base : IdentBase,
    ue_type    : UeType,
    event      : Box<dyn UpdatingEvent>,
}

impl UpdateEventEntry {
    pub fn new(event: Box<dyn UpdatingEvent>) -> Self {
        let ue_type    = event.get_type();
        let ident_base = IdentBase::new(false, ue_type.prefix());
        Self { ident_base, ue_type, event }
    }

    /// Call this after the entry has been inserted into the arena so that
    /// `ident_base.arena_handle` has already been set by `ArenaGroup::insert`.
    pub fn get_ident(&self) -> UpdateEventIdent {
        UpdateEventIdent { ident_base: self.ident_base, ue_type: self.ue_type }
    }

    pub fn get_event    (&self)     -> &dyn UpdatingEvent     { &*self.event      }
    pub fn get_event_mut(&mut self) -> &mut dyn UpdatingEvent { &mut *self.event  }
}

impl Identifiable for UpdateEventEntry {
    fn get_ident_base    (&self)     -> &IdentBase     { &self.ident_base }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { &mut self.ident_base }
    fn build_unique_name (&mut self) -> &str {
        let id   = self.ident_base.get_global_id();
        let name = format!("{}_{}", self.ue_type.prefix(), id);
        self.ident_base.set_name(&name);
        self.ident_base.get_name()
    }
}

// ---------------------------------------------------------------------------
// UpdateEventIdent — lightweight handle held by UpdatePool and callers
// ---------------------------------------------------------------------------

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct UpdateEventIdent {
    ident_base : IdentBase,
    ue_type    : UeType,
}

impl UpdateEventIdent {
    pub fn get_ue_type(&self) -> UeType { self.ue_type }
}

impl Identifiable for UpdateEventIdent {
    fn get_ident_base    (&self)     -> &IdentBase     { &self.ident_base }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { &mut self.ident_base }
    fn build_unique_name (&mut self) -> &str {
        let id   = self.ident_base.get_global_id();
        let name = format!("{}_{}", self.ue_type.prefix(), id);
        self.ident_base.set_name(&name);
        self.ident_base.get_name()
    }
}
