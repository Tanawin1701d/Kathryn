use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::update_event::UpdatingEvent;

pub struct UpdatePool {
    events: Vec<Box<dyn UpdatingEvent>>,
}

impl UpdatePool {
    pub fn new() -> Self {
        Self { events: Vec::new() }
    }

    pub fn add_update_event(&mut self, event: Box<dyn UpdatingEvent>) {
        self.events.push(event);
    }

    pub fn get_update_events(&self) -> &Vec<Box<dyn UpdatingEvent>> {
        &self.events
    }

    pub fn size    (&self) -> usize { self.events.len()   }
    pub fn is_empty(&self) -> bool  { self.events.is_empty() }

    pub fn sort_events(&mut self) {
        self.events.sort_by(|a, b| {
            a.get_priority().cmp(&b.get_priority())
                .then(a.get_sub_priority().cmp(&b.get_sub_priority()))
        });
    }

    /// Returns None if there are no events.
    pub fn get_clock_mode(&self) -> Option<ClockMode> {
        self.events.first().map(|e| e.get_clk_mode())
    }

    pub fn is_clock_mode_consistent(&self) -> bool {
        let Some(first_mode) = self.get_clock_mode() else { return true; };
        self.events.iter().all(|e| e.get_clk_mode() == first_mode)
    }

    pub fn clean(&mut self) {
        self.events.clear();
    }
}