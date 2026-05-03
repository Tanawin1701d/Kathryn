use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::update_event_ident::UpdateEventIdent;
use crate::model::model_arena::ModelArena;

#[derive(Default)]
pub struct UpdatePool {
    events: Vec<UpdateEventIdent>,
}

impl UpdatePool {
    pub fn new() -> Self {
        Self { events: Vec::new() }
    }

    pub fn add_update_event(&mut self, ident: UpdateEventIdent) {
        self.events.push(ident);
    }

    pub fn get_update_events(&self) -> &Vec<UpdateEventIdent> {
        &self.events
    }

    pub fn size    (&self) -> usize { self.events.len()      }
    pub fn is_empty(&self) -> bool  { self.events.is_empty() }

    pub fn sort_events(&mut self, arena: &ModelArena) {
        self.events.sort_by(|a, b| {
            let ca = arena.get_ue_common(a);
            let cb = arena.get_ue_common(b);
            ca.get_priority().cmp(&cb.get_priority())
             .then(ca.get_sub_priority().cmp(&cb.get_sub_priority()))
        });
    }

    pub fn get_clock_mode(&self, arena: &ModelArena) -> Option<ClockMode> {
        self.events.first().map(|i| arena.get_ue_common(i).get_clk_mode())
    }

    pub fn is_clock_mode_consistent(&self, arena: &ModelArena) -> bool {
        let Some(first_mode) = self.get_clock_mode(arena) else { return true; };
        self.events.iter().all(|i| arena.get_ue_common(i).get_clk_mode() == first_mode)
    }

    pub fn clean(&mut self) {
        self.events.clear();
    }
}
