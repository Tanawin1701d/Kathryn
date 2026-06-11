use crate::model::common::identifier::{IdentBase, Identifiable};
use crate::model::hw_component::common::update_event::UeType;

// ---------------------------------------------------------------------------
// UpdateEventIdent — lightweight Copy handle held by UpdatePool and callers.
// The arena_handle inside ident_base routes to the correct typed arena field
// in ModelArena; ue_type selects which field to look in.
// ---------------------------------------------------------------------------

#[derive(Clone, Copy, Debug, PartialEq, Eq, Default)]
pub struct UpdateEventIdent {
    ident_base : IdentBase,
    ue_type    : UeType,
}

impl UpdateEventIdent {
    pub fn new(ident_base: IdentBase, ue_type: UeType) -> Self {
        Self { ident_base, ue_type }
    }

    pub fn get_ue_type(&self) -> UeType { self.ue_type }
}

impl Identifiable for UpdateEventIdent {
    fn get_ident_base    (&self)     -> &IdentBase     { &self.ident_base }
    fn get_ident_base_mut(&mut self) -> &mut IdentBase { &mut self.ident_base }
    fn build_unique_name (&mut self) -> &str {
        let pre  = self.ue_type.prefix();
        let id   = self.ident_base.get_global_id();
        let name = format!("{pre}_{id}");
        self.ident_base.set_abs_name(&name);
        self.ident_base.get_abs_name()
    }
}
