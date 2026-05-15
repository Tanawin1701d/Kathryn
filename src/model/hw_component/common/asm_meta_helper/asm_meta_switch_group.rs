use crate::model::controller::clock_mode::ClockMode;
use crate::model::controller::clock_mode::ClockMode::ClkUnused;
use crate::model::hw_component::common::assign_meta::AssignMeta;
use crate::model::hw_component::common::hcp_ident::HcpIdent;
use crate::model::hw_component::common::update_event::UeSwitch;
use crate::model::hw_component::common::update_event_ident::UpdateEventIdent;
use crate::model::model_arena::ModelArena;

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct AssignMetaSwitchGroup {
    target_hwc   : HcpIdent,
    state_i      : HcpIdent,
    match_vals   : Vec<i32>,
    update_events: Vec<Option<UpdateEventIdent>>,
}

impl AssignMetaSwitchGroup {
    pub fn new(target_hwc     : HcpIdent,
               state_i        : HcpIdent,
               first_match_val: i32,
               first_ue       : Option<UpdateEventIdent>) -> Self {
        Self {
            target_hwc,
            state_i,
            match_vals   : vec![first_match_val],
            update_events: vec![first_ue],
        }
    }

    pub fn is_joinable(&self, arena: &mut ModelArena, asm: &AssignMeta) -> bool {
        self.target_hwc == asm.get_target_hw() &&
            self.update_events.iter()
                .find_map(|&ue| ue)
                .map_or(false, |ref_ue| arena.is_ue_joinable(ref_ue, asm.get_pre_update_event()))
    }

    pub fn try_push_event(&mut self, arena        : &mut ModelArena,
                                     match_val    : i32,
                                     asm_candidates: &mut Vec<AssignMeta>) -> bool {
        // before push_event, you should check with is_joinable
        assert_eq!(self.match_vals.len(), self.update_events.len(), "match_vals and update_events length must match");

        let pos = asm_candidates.iter().position(|asm_can| self.is_joinable(arena, asm_can));
        let ue_i = pos.map(|idx| asm_candidates.remove(idx).get_pre_update_event());
        self.match_vals.push(match_val);
        self.update_events.push(ue_i);
        pos.is_some()
    }

    fn get_ref_priority(&self, arena: &mut ModelArena) -> i32 {
        self.update_events.iter()
            .find_map(|&ue| ue)
            .map_or(0, |ue| arena.get_ue_common(&ue).get_priority())
    }

    fn get_ref_clk_mode(&self, arena: &mut ModelArena) -> ClockMode {
        self.update_events.iter()
            .find_map(|&ue| ue)
            .map_or(ClkUnused, |ue| arena.get_ue_common(&ue).get_clk_mode())
    }

    pub fn gen_update_event(&self, arena: &mut ModelArena) -> UpdateEventIdent {
        let ue_len = self.update_events.len();
        let mv_len = self.match_vals.len();
        assert_eq!(
            ue_len, mv_len,
            "match_vals length ({mv_len}) must equal update_events length ({ue_len})"
        );

        let mut ues = UeSwitch::new(self.state_i);
        for (&match_val, &ue_i) in self.match_vals.iter().zip(self.update_events.iter()) {
            let priority = self.get_ref_priority(arena);
            let clk_mode = self.get_ref_clk_mode(arena);
            ues.add_sub_stmt(match_val, ue_i, priority, clk_mode);
        }
        arena.insert_ue_switch(ues)
    }

    pub fn gen_assign_meta(&self, arena: &mut ModelArena) -> AssignMeta {
        let ue_i = self.gen_update_event(arena);
        AssignMeta::new(self.target_hwc, ue_i, ClkUnused)
    }
}

impl Default for AssignMetaSwitchGroup {
    fn default() -> Self {
        Self {
            target_hwc   : HcpIdent::default(),
            state_i      : HcpIdent::default(),
            match_vals   : Vec::new(),
            update_events: Vec::new(),
        }
    }
}

pub struct AssignMetaSwitchPool {
    asm_pool: Vec<AssignMetaSwitchGroup>,
}

impl AssignMetaSwitchPool {
    pub fn add_new_group(&mut self, asm: &AssignMeta, state_i: HcpIdent, match_val: i32) {
        let group = AssignMetaSwitchGroup::new(
            asm.get_target_hw(), state_i, match_val, Some(asm.get_pre_update_event()),
        );
        self.asm_pool.push(group);
    }

    pub fn insert_asms(&mut self, arena         : &mut ModelArena,
                                  state_i       : HcpIdent,
                                  match_val     : i32,
                                  asm_candidates: &Vec<AssignMeta>) {
        let mut candidates_copy = asm_candidates.clone();
        for group in &mut self.asm_pool {
            group.try_push_event(arena, match_val, &mut candidates_copy);
        }
        candidates_copy.iter()
            .for_each(|asm| self.add_new_group(asm, state_i, match_val));
    }

    pub fn gen_assign_metas(&self, arena: &mut ModelArena) -> Vec<AssignMeta> {
        self.asm_pool.iter().map(|group| group.gen_assign_meta(arena)).collect()
    }
}
