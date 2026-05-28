use std::collections::{HashMap, HashSet};
use crate::model::controller::clock_mode::ClockMode;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::update_event::UpdatingEvent;
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

    pub fn get_clk_src_i(&self, arena: &ModelArena) -> Option<HcpIdent> {
        self.events.first().and_then(|i| arena.get_ue_common(i).get_clk_src_i())
    }

    pub fn is_clk_src_consistent(&self, arena: &ModelArena) -> bool {
        let first_src = self.events.first().map(|i| arena.get_ue_common(i).get_clk_src_i());
        let Some(first_src) = first_src else { return true; };
        self.events.iter().all(|i| arena.get_ue_common(i).get_clk_src_i() == first_src)
    }

    pub fn clean(&mut self) {
        self.events.clear();
    }

    pub fn remap_dep_hcps(&self, map: &HashMap<HcpIdent, HcpIdent>, arena: &mut ModelArena) {
        let ue_idents: Vec<UpdateEventIdent> = self.events.clone();
        for ue_i in ue_idents {
            let mut ue = arena.take_ue(ue_i);
            ue.remap_dep_hcps(map, arena);
            arena.replace_back_ue(ue);
        }
    }

    pub fn gather_dep_hcps(&self, arena: &mut ModelArena, out: &mut HashSet<HcpIdent>) {
        let ue_idents: Vec<UpdateEventIdent> = self.events.clone();
        for ue_i in ue_idents {
            let ue = arena.take_ue(ue_i);
            ue.gather_dep_hcps(arena, out);
            arena.replace_back_ue(ue);
        }
    }
}
