use crate::backends::verilog::hw_component::common::update_event_vb::VerilogUpdateEvent;
use crate::model::hw_component::common::update_event::UeType;
use crate::model::hw_component::common::update_event_ident::UpdateEventIdent;
use crate::model::model_arena::ModelArena;

// ---- VerilogUpdateEvent arena extension ----

impl ModelArena {

    /// Take an update event as a `VerilogUpdateEvent` trait object.
    /// ONE match lives here — `transpile_ue` stays match-free.
    /// When a new UeType is added, add one arm here and `impl VerilogUpdateEvent`
    /// for the new type; nothing else changes.
    pub fn take_ue_vb(&mut self, ident: UpdateEventIdent) -> Box<dyn VerilogUpdateEvent> {
        match ident.get_ue_type() {
            UeType::Basic  => Box::new(self.take_ue_basic (ident)),
            UeType::Grp    => Box::new(self.take_ue_grp   (ident)),
            UeType::Cond   => Box::new(self.take_ue_cond  (ident)),
            UeType::Switch => Box::new(self.take_ue_switch (ident)),
            UeType::Untype => panic!("take_ue_vb: UeType::Untype"),
        }
    }
}
